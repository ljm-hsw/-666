# AGENTS.md

## Purpose and Scope

This is the repository-level instruction set for this workspace. Read it before inspecting or changing files.

- These instructions apply to the entire directory tree rooted here.
- A more deeply nested `AGENTS.md` may add or override rules for its subtree.
- Explicit system, developer, and user instructions take precedence over this file.
- Do not assume a language, framework, build system, or deployment target. Discover them from the repository first.

## Repository Map

- `AGENTS.md`: stable repository-wide workflow and safety rules.
- `DEVELOPMENT.md`: five-person collaboration, branch, review, test, and delivery rules.
- `CREDITS.md`: third-party runtime asset provenance, versions, licenses, and hashes.
- `CONTEXT.md`: shared product-domain vocabulary and invariants.
- `docs/README.md`: documentation map and source-of-truth order.
- `docs/PROJECT_PLAN.md`: P0-P4 milestone gates and issue dependency map.
- `docs/DESIGN.md`: core architecture, state flow, contracts, and subsystem design.
- `docs/CHARACTER_DIALOGUE_PLAN.md`: P4 placeholder character specification, lightweight dialogue, bounded indoor navigation, collision, triggers, and issue sequence.
- `docs/SCENE_ASSET_PRODUCTION_PLAN.md`: modular imagegen asset batches, manual scene compositing, collision handoff, and visual approval gates.
- `docs/VISUAL_PROTOTYPE.md`: unapproved P0 visual variants and candidate asset rules.
- `docs/adr/`: accepted architectural decisions and their rationale.
- `docs/agents/`: repository-local issue tracker, triage, and domain-doc conventions.
- `.scratch/`: local PRDs and implementation issues, grouped by feature.
- `skills/`: repository-local agent skills copied for this workspace only.
- `src/`: C++17 game source, with raylib-free rules and configuration under `src/core/`.
- `tests/`: headless doctest coverage registered through CTest.
- `assets/`: project-owned runtime resources; final visual and audio specifications require HITL approval.
- `third_party/`: locked raylib/doctest sources and their licenses; builds must not download replacements.
- `.github/workflows/`: Windows and macOS continuous-integration checks.

Update this map when the repository gains stable top-level source, test, documentation, or tooling directories. Do not put temporary task status here.

## Quick Start

Before making changes:

1. Read this file and any nested `AGENTS.md` governing the target files.
2. Inspect `git status --short`; preserve unrelated and user-authored changes.
3. Use `rg --files` and relevant manifests, README files, CI configuration, and tests to discover the project structure and commands.
4. Read only the smallest relevant set of documentation and code.
5. Read `DEVELOPMENT.md` before implementation or review work involving team delivery.
6. State assumptions, acceptance criteria, non-goals, and the validation plan for non-trivial work.

If sources conflict, prefer current executable code, tests, and configuration over active task contracts, current documentation and runbooks, and finally historical notes. Surface material conflicts instead of silently choosing one.

## Think Before Coding

- Do not hide uncertainty. State assumptions that affect behavior or scope.
- If multiple interpretations would produce materially different outcomes, present them and ask before committing to one.
- If the ambiguity is low-risk and reversible, make the smallest reasonable assumption, state it, and continue.
- Prefer the simplest approach that satisfies the request; explain meaningful tradeoffs.
- For multi-step work, use a short goal-driven plan in the form `step -> verification`.
- Do not implement speculative requirements.

## Implementation Principles

- Write the minimum code needed to satisfy the acceptance criteria.
- Avoid abstractions, configurability, and fallback paths that have only hypothetical use.
- Match existing style, naming, and architecture.
- Do not refactor, reformat, or clean up unrelated code.
- Every changed line must trace directly to the request or to a necessary consequence of it.
- Remove imports, variables, files, or functions made obsolete by your own change; leave pre-existing dead code alone and report it separately.
- Preserve baseline behavior unless the task explicitly changes it.
- Keep changes scoped and reversible.

## Workflow by Task Type

- **Review:** remain read-only. Report evidence-backed findings by severity; do not implement fixes unless asked.
- **Diagnose:** reproduce or inspect first, then form and test hypotheses. Explain the cause; do not modify files unless the request includes a fix.
- **Fix:** add or identify a failing reproduction when practical, make the smallest correction, and rerun the relevant check.
- **Feature:** define observable acceptance criteria, implement only the requested behavior, and add focused tests.
- **Refactor:** establish passing behavior before the change and verify equivalent behavior afterward.
- **Documentation:** verify claims against current code, configuration, or runtime evidence when feasible.

For experimental or performance work, report correctness, experiment validity, and performance evidence separately. Do not convert a smoke test or configuration claim into a readiness or speed claim.

## Validation

- Discover canonical commands from repository manifests, CI workflows, scripts, and existing documentation; do not invent project commands.
- Run the narrowest relevant tests, type checks, linters, compile checks, or script syntax checks after editing.
- Add or update tests when behavior changes, unless the repository has no suitable test framework or the user excludes tests.
- Expand validation only when risk or failures justify it; do not run broad formatting over unrelated files.
- Review `git diff --check`, `git diff --stat`, the full task-related diff, and `git status --short` before finishing.
- If a check cannot run, report the exact command, reason, and remaining risk. Never claim validation that did not execute.

## Safety and Hard Boundaries

Pause and ask before:

- Adding or upgrading external dependencies, system packages, or generated lockfiles not required by an explicit request.
- Changing system, network, authentication, permission, secret-storage, logging, deployment, or shared-service configuration.
- Writing to shared data, caches, remote services, or directories outside this repository.
- Starting long-running jobs, expensive experiments, deployments, or externally visible actions.
- Deleting user data, overwriting artifacts, rewriting Git history, or running destructive commands.
- Changing public interfaces, persistent data formats, or defaults with broad compatibility impact when the requirement is unclear.

Never expose or commit secrets, credentials, private keys, tokens, personal data, large generated outputs, caches, logs, virtual environments, model weights, or checkpoints. Redact sensitive values from reports and command output.

Repository-local configuration and skills must remain local. Do not modify or install anything under user-level or global directories such as `~/.codex` unless the user explicitly requests that exact scope.

## Git and Workspace Hygiene

- Treat a dirty worktree as user-owned state. Do not discard, overwrite, stage, or reformat unrelated changes.
- Do not create or switch branches, commit, amend, push, open pull requests, or tag releases unless explicitly asked.
- When Git actions are requested, include only task-related files and verify the resulting status.
- Do not use destructive Git commands unless the user explicitly authorizes the exact operation and scope.
- Prefer non-interactive commands and preserve a clear rollback path.

## Documentation and Planning

- Keep this file concise and stable: use it as a map plus hard rules, not as an encyclopedia or activity log.
- Put detailed architecture, runbooks, product requirements, and experiment procedures in focused repository documentation when those directories exist.
- For a substantial new idea, clarify and record the plan before implementation; split it into issues only after the boundary is settled.
- Distinguish documented intent from behavior proven by tests, runtime inspection, or artifacts.
- Do not record secrets, full login commands, volatile server state, or large logs in documentation.

## Agent skills

### Issue tracker

Issues and PRDs use local Markdown under `.scratch/`. See `docs/agents/issue-tracker.md`.

### Triage labels

Use the default five-role vocabulary. See `docs/agents/triage-labels.md`.

### Domain docs

This is a single-context repository using root `CONTEXT.md` and `docs/adr/`. See `docs/agents/domain.md`.

### Local skills

Skills under `skills/` are available only to this repository.

- Read the relevant `skills/<name>/SKILL.md` completely before using a skill.
- Follow referenced resources selectively; do not load unrelated skill material.
- Prefer bundled scripts and templates over re-creating them.
- If skill instructions conflict with an explicit user request, follow the user request and state the conflict.
- Do not copy, link, or install these skills into user or global skill directories unless explicitly asked.

## Completion Report

Keep the final report concise and factual:

- What changed and which files changed.
- What validation ran and its result.
- What was not validated and why.
- Remaining risks, assumptions, or decisions requiring user input.
