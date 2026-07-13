#!/usr/bin/env python3
"""Split transparent imagegen sheets into numbered standalone PNG pieces.

This is an art-source utility, not a runtime dependency. Run it in the local
``lab`` Conda environment, which provides Pillow, NumPy, and SciPy.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont
from scipy import ndimage


ALPHA_THRESHOLD = 32
MIN_AREA = 80
GROUPING_RADIUS = 0
PADDING = 4


@dataclass(frozen=True)
class PieceRecord:
    location: str
    kind: str
    index: int
    source: str
    output: str
    left: int
    top: int
    right: int
    bottom: int
    width: int
    height: int
    alpha_pixels: int
    sha256: str


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def find_components(
    image: Image.Image,
) -> tuple[np.ndarray, list[tuple[int, int, int, int, int, int]]]:
    rgba = np.asarray(image.convert("RGBA"))
    alpha_mask = rgba[:, :, 3] >= ALPHA_THRESHOLD

    # Keep disconnected furniture and props independent. The chroma-key helper
    # already produces continuous alpha edges, so no dilation is needed here.
    grouped_mask = alpha_mask
    if GROUPING_RADIUS:
        grouped_mask = ndimage.binary_dilation(
            alpha_mask,
            structure=np.ones((3, 3), dtype=bool),
            iterations=GROUPING_RADIUS,
        )
    labels, count = ndimage.label(
        grouped_mask,
        structure=np.ones((3, 3), dtype=np.uint8),
    )

    components: list[tuple[int, int, int, int, int, int]] = []
    for label_index in range(1, count + 1):
        ys, xs = np.where((labels == label_index) & alpha_mask)
        if xs.size < MIN_AREA:
            continue
        left = int(xs.min())
        top = int(ys.min())
        right = int(xs.max()) + 1
        bottom = int(ys.max()) + 1
        components.append((label_index, left, top, right, bottom, int(xs.size)))

    # Stable reading order: row first, then column. A 24-pixel row bucket keeps
    # objects with slightly different top edges in the same visual row.
    components.sort(key=lambda box: (box[2] // 24, box[1], box[2]))
    return labels, components


def crop_piece(
    source: Image.Image,
    labels: np.ndarray,
    bounds: tuple[int, int, int, int, int, int],
) -> tuple[Image.Image, tuple[int, int, int, int]]:
    label_index, left, top, right, bottom, _ = bounds
    crop_box = (
        max(0, left - PADDING),
        max(0, top - PADDING),
        min(source.width, right + PADDING),
        min(source.height, bottom + PADDING),
    )
    piece = source.crop(crop_box).convert("RGBA")
    pixels = np.asarray(piece).copy()
    component_mask = labels[crop_box[1] : crop_box[3], crop_box[0] : crop_box[2]] == label_index
    pixels[~component_mask] = (0, 0, 0, 0)
    pixels[pixels[:, :, 3] == 0] = (0, 0, 0, 0)
    return Image.fromarray(pixels, mode="RGBA"), crop_box


def checkerboard(size: tuple[int, int], tile: int = 16) -> Image.Image:
    width, height = size
    background = Image.new("RGB", size, (216, 216, 216))
    draw = ImageDraw.Draw(background)
    for y in range(0, height, tile):
        for x in range(0, width, tile):
            if (x // tile + y // tile) % 2:
                draw.rectangle(
                    (x, y, min(x + tile - 1, width - 1), min(y + tile - 1, height - 1)),
                    fill=(174, 174, 174),
                )
    return background


def load_font(size: int) -> ImageFont.ImageFont:
    candidates = (
        Path("/System/Library/Fonts/Supplemental/Arial Bold.ttf"),
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"),
    )
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size)
    return ImageFont.load_default()


def write_contact_sheet(piece_paths: list[Path], output: Path, title: str) -> None:
    columns = 5
    cell_width = 220
    cell_height = 220
    label_height = 30
    header_height = 48
    rows = math.ceil(len(piece_paths) / columns)
    canvas = Image.new(
        "RGB",
        (columns * cell_width, header_height + rows * (cell_height + label_height)),
        (35, 37, 41),
    )
    draw = ImageDraw.Draw(canvas)
    title_font = load_font(25)
    label_font = load_font(16)
    draw.text((14, 9), title, font=title_font, fill=(245, 238, 216))

    for position, path in enumerate(piece_paths):
        row, column = divmod(position, columns)
        x = column * cell_width
        y = header_height + row * (cell_height + label_height)
        piece = Image.open(path).convert("RGBA")
        scale = min(
            1.0,
            (cell_width - 20) / piece.width,
            (cell_height - 20) / piece.height,
        )
        preview = piece.resize(
            (max(1, round(piece.width * scale)), max(1, round(piece.height * scale))),
            Image.Resampling.NEAREST,
        )
        cell = checkerboard((cell_width, cell_height))
        cell.paste(
            preview,
            ((cell_width - preview.width) // 2, (cell_height - preview.height) // 2),
            preview,
        )
        canvas.paste(cell, (x, y))
        draw.text(
            (x + 8, y + cell_height + 5),
            f"piece_{position + 1:03d}",
            font=label_font,
            fill=(245, 238, 216),
        )

    canvas.save(output, optimize=True)


def parse_sheet_name(path: Path) -> tuple[str, str]:
    location = path.parents[1].name
    prefix = f"{location}_"
    suffix = "_candidate_v01.png"
    if not path.name.startswith(prefix) or not path.name.endswith(suffix):
        raise ValueError(f"Unexpected selected-sheet name: {path}")
    kind = path.name[len(prefix) : -len(suffix)]
    if kind not in {"architecture", "furniture", "props"}:
        raise ValueError(f"Unexpected sheet kind: {kind}")
    return location, kind


def extract_sheet(path: Path, scene_root: Path, overwrite: bool) -> list[PieceRecord]:
    location, kind = parse_sheet_name(path)
    output_dir = scene_root / location / "pieces" / kind
    output_dir.mkdir(parents=True, exist_ok=True)
    if any(output_dir.iterdir()) and not overwrite:
        raise FileExistsError(f"Refusing to overwrite non-empty directory: {output_dir}")

    source = Image.open(path).convert("RGBA")
    labels, components = find_components(source)
    records: list[PieceRecord] = []
    piece_paths: list[Path] = []

    for index, bounds in enumerate(components, start=1):
        piece, crop_box = crop_piece(source, labels, bounds)
        output = output_dir / f"{location}_{kind}_piece_{index:03d}_candidate_v01.png"
        piece.save(output, optimize=True)
        alpha_pixels = int(np.count_nonzero(np.asarray(piece)[:, :, 3] >= ALPHA_THRESHOLD))
        records.append(
            PieceRecord(
                location=location,
                kind=kind,
                index=index,
                source=str(path.relative_to(scene_root)),
                output=str(output.relative_to(scene_root)),
                left=crop_box[0],
                top=crop_box[1],
                right=crop_box[2],
                bottom=crop_box[3],
                width=piece.width,
                height=piece.height,
                alpha_pixels=alpha_pixels,
                sha256=sha256(output),
            )
        )
        piece_paths.append(output)

    write_contact_sheet(
        piece_paths,
        output_dir / f"{location}_{kind}_pieces_contact_sheet.png",
        f"{location.upper()} {kind.upper()} PIECES",
    )
    return records


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent,
        help="scene_kits directory",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="replace only the deterministic files produced by this script",
    )
    args = parser.parse_args()
    scene_root = args.root.resolve()
    sheets = sorted(scene_root.glob("*/selected/*_candidate_v01.png"))
    if not sheets:
        raise FileNotFoundError("No selected candidate sheets found")

    records: list[PieceRecord] = []
    for sheet in sheets:
        records.extend(extract_sheet(sheet, scene_root, args.overwrite))

    json_path = scene_root / "individual_pieces_manifest.json"
    csv_path = scene_root / "individual_pieces_manifest.csv"
    json_path.write_text(
        json.dumps([asdict(record) for record in records], ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    with csv_path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=PieceRecord.__dataclass_fields__.keys())
        writer.writeheader()
        writer.writerows(asdict(record) for record in records)

    print(f"Extracted {len(records)} pieces from {len(sheets)} sheets")
    print(f"Wrote {json_path}")
    print(f"Wrote {csv_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
