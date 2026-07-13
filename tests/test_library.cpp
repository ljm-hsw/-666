#include <doctest/doctest.h>

#include "locations/library_data.hpp"
#include "locations/library_rules.hpp"

namespace {

pixel_town::library::LibraryData create_test_data() {
    pixel_town::library::LibraryData data;

    data.categories = {
        {"history", "历史", "记录人类文明发展历程", ""},
        {"science", "科学", "探索自然规律", ""},
        {"literature", "文学", "人类情感表达", ""},
        {"art", "艺术", "视觉美学创造", ""},
        {"technology", "技术", "智慧与工具结合", ""},
    };

    data.books = {
        {"book1", "秦始皇传", "history", false, 0, 0},
        {"book2", "物理奥秘", "science", false, 0, 0},
        {"book3", "红楼梦", "literature", false, 0, 0},
        {"book4", "名画鉴赏", "art", false, 0, 0},
        {"book5", "编程入门", "technology", false, 0, 0},
        {"book6", "三国风云", "history", false, 0, 0},
        {"book7", "化学探索", "science", false, 0, 0},
        {"book8", "唐诗宋词", "literature", false, 0, 0},
    };

    data.shelves = {
        {"shelf1", "history", "历史书架", 0, 0, 8},
        {"shelf2", "science", "科学书架", 0, 0, 8},
        {"shelf3", "literature", "文学书架", 0, 0, 8},
        {"shelf4", "art", "艺术书架", 0, 0, 8},
        {"shelf5", "technology", "技术书架", 0, 0, 8},
    };

    data.dialogue.greeting = "欢迎来到图书馆";
    data.dialogue.introduction = "书架按类别分的";
    data.dialogue.familiar_talk = "窗台晒着借书卡";
    data.dialogue.close_friend_talk = "管理员递给你一杯热茶";
    data.dialogue.old_map_reveal = "掉出一张旧集市地图";
    data.dialogue.borrow_card = "借书卡盒子里多了几张新记录";

    data.plot_events = {
        {"test_plot", "测试剧情", "测试描述", 2, 30, false},
    };

    data.welcome_message = "欢迎来到小镇图书馆";
    data.work_intro = "整理地上散落的书籍";

    return data;
}

}  // namespace

TEST_CASE("Library data loading - valid format") {
    pixel_town::library::LibraryData data = create_test_data();
    CHECK(data.categories.size() == 5);
    CHECK(data.books.size() == 8);
    CHECK(data.shelves.size() == 5);
    CHECK(data.categories[0].id == "history");
    CHECK(data.categories[0].name == "历史");
    CHECK(data.categories[0].description == "记录人类文明发展历程");
    CHECK(data.books[0].title == "秦始皇传");
    CHECK(data.books[0].category_id == "history");
    CHECK(data.shelves[0].id == "shelf1");
    CHECK(data.shelves[0].category_id == "history");
    CHECK(data.shelves[0].name == "历史书架");
    CHECK(data.dialogue.greeting == "欢迎来到图书馆");
    CHECK(data.plot_events.size() == 1);
    CHECK(data.plot_events[0].id == "test_plot");
    CHECK(data.welcome_message == "欢迎来到小镇图书馆");
}

TEST_CASE("Library rule engine - start session") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    CHECK(engine.is_session_active());
    CHECK(!engine.is_session_completed());
    CHECK(engine.get_session_state().correct_count == 0);
    CHECK(engine.get_session_state().wrong_count == 0);
    CHECK(!engine.get_scattered_books().empty());
}

TEST_CASE("Library rule engine - pick up and place book correctly") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    const auto& books = engine.get_scattered_books();
    if (!books.empty()) {
        const std::string book_id = books[0].book_id;
        const std::string category_id = books[0].category_id;

        engine.pick_up_book(book_id);
        CHECK(engine.is_holding_book());
        CHECK(engine.get_held_book_category() == category_id);

        std::string correct_shelf_id;
        for (const auto& shelf : engine.get_shelves()) {
            if (shelf.category_id == category_id) {
                correct_shelf_id = shelf.id;
                break;
            }
        }

        bool result = engine.place_book_on_shelf(correct_shelf_id);
        CHECK(result);
        CHECK(engine.was_last_answer_correct());
        CHECK(!engine.is_holding_book());
        CHECK(engine.get_session_state().correct_count == 1);
        CHECK(engine.get_session_state().placed_count == 1);
    }
}

TEST_CASE("Library rule engine - place book incorrectly") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    const auto& books = engine.get_scattered_books();
    if (!books.empty()) {
        const std::string book_id = books[0].book_id;
        const std::string category_id = books[0].category_id;

        engine.pick_up_book(book_id);

        std::string wrong_shelf_id;
        for (const auto& shelf : engine.get_shelves()) {
            if (shelf.category_id != category_id) {
                wrong_shelf_id = shelf.id;
                break;
            }
        }

        bool result = engine.place_book_on_shelf(wrong_shelf_id);
        CHECK(!result);
        CHECK(!engine.was_last_answer_correct());
        CHECK(!engine.is_holding_book());
        CHECK(engine.get_session_state().wrong_count == 1);
    }
}

TEST_CASE("Library rule engine - give up") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    engine.give_up();

    CHECK(engine.is_session_completed());
    CHECK(!engine.is_session_active());

    pixel_town::library::ActionResult result = engine.finish_session();
    CHECK(result.gave_up);
    CHECK(result.stamina_change == 0);
    CHECK(result.knowledge_change == 0);
    CHECK(result.reputation_change == 0);
}

TEST_CASE("Library rule engine - action result calculation") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    pixel_town::library::DailyContext context;
    context.day = 1;
    context.random_seed = 12345;
    engine.start_session(context);

    const int expected_placed = static_cast<int>(engine.get_scattered_books().size());
    
    while (!engine.get_scattered_books().empty()) {
        const auto& books = engine.get_scattered_books();
        const std::string book_id = books[0].book_id;
        const std::string category_id = books[0].category_id;
        
        engine.pick_up_book(book_id);
        std::string correct_shelf_id;
        for (const auto& shelf : engine.get_shelves()) {
            if (shelf.category_id == category_id) {
                correct_shelf_id = shelf.id;
                break;
            }
        }
        engine.place_book_on_shelf(correct_shelf_id);
    }

    pixel_town::library::ActionResult result = engine.finish_session();

    CHECK(result.completed);
    CHECK(result.stamina_change == -config.stamina_cost);
    CHECK(result.knowledge_change == expected_placed * config.correct_knowledge_reward);
    CHECK(result.reputation_change == expected_placed * config.correct_reputation_reward + expected_placed * config.combo_reputation_bonus);
    CHECK(result.mood_change == config.base_mood_change + expected_placed * config.correct_mood_bonus);
}

TEST_CASE("Library rule engine - npc relationship progression") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    engine.update_npc_relationship(0, 0);
    CHECK(engine.get_npc_interaction().relationship ==
          pixel_town::library::NpcRelationship::stranger);

    engine.update_npc_relationship(30, 2);
    CHECK(engine.get_npc_interaction().relationship ==
          pixel_town::library::NpcRelationship::familiar);

    engine.update_npc_relationship(50, 5);
    CHECK(engine.get_npc_interaction().borrow_card_given);

    engine.update_npc_relationship(80, 8);
    CHECK(engine.get_npc_interaction().close_friend_unlocked);
}

TEST_CASE("Library rule engine - plot events") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    CHECK(!engine.has_pending_plot_event());

    engine.check_plot_events(25, 1);
    CHECK(!engine.has_pending_plot_event());

    engine.check_plot_events(30, 2);
    CHECK(engine.has_pending_plot_event());
    CHECK(engine.get_npc_interaction().current_plot_title == "测试剧情");

    engine.trigger_plot_event("test_plot");
    CHECK(!engine.has_pending_plot_event());
}

TEST_CASE("Library rule engine - map reveal conditions") {
    pixel_town::library::LibraryData data = create_test_data();
    pixel_town::library::LibraryConfig config =
        pixel_town::library::default_library_config();
    pixel_town::library::LibraryRuleEngine engine(data, config);

    CHECK(!engine.should_reveal_map(25, 1));
    CHECK(engine.should_reveal_map(30, 2));

    engine.reveal_map();
    CHECK(engine.get_npc_interaction().map_revealed);
    CHECK(!engine.should_reveal_map(30, 2));
}
