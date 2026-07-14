#pragma once

#include <memory>
#include <string>

#include "core/game_session.hpp"
#include "locations/library_data.hpp"
#include "locations/library_organizing.hpp"
#include "locations/library_rules.hpp"
#include "locations/library_runtime_contract.hpp"

namespace pixel_town {

enum class LibraryOpenStatus {
    opened,
    denied,
    already_active,
};

struct LibraryOpenResult {
    LibraryOpenStatus status{LibraryOpenStatus::denied};
    std::string message;
};

enum class LibraryStepStatus {
    unchanged,
    changed,
    settled,
    rejected,
};

struct LibraryStepResult {
    LibraryStepStatus status{LibraryStepStatus::unchanged};
    std::string notice;
};

class LibraryRuntime {
public:
    [[nodiscard]] LibraryOpenResult open(GameSession& session,
                                         const library::LibraryData& data);
    [[nodiscard]] LibraryStepResult step(GameSession& session,
                                         const LibraryIntent& intent);
    [[nodiscard]] LibraryPresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    bool active_{false};
    LibraryRuntimeMode mode_{LibraryRuntimeMode::selection};
    library::LibraryData data_;
    std::unique_ptr<library::LibraryRuleEngine> reader_;
    std::unique_ptr<library::LibraryOrganizingSession> organizing_;
    int visits_{0};
    int active_result_id_{0};
    std::string feedback_;

    void close();
    [[nodiscard]] LibraryStepResult settle(
        GameSession& session, const library::LibraryWorkResult& work_result);
};

}  // namespace pixel_town
