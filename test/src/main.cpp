#include <chrono>
#include <thread>

#include <so_5/all.hpp>

using namespace std::string_literals;

// Message to be sent to an agent.
struct hello {
    std::string greeting_;
};

// Demo agent.
class demo final : public so_5::agent_t {
    const std::string name_;

    void on_hello(mhood_t<hello> cmd) {
        if (name_=="Bob")
            std::this_thread::sleep_for(std::chrono::seconds(10));

        std::cout << name_ << ": greeting received: "
                << cmd->greeting_ << std::endl;

        // Now agent can finish its work.
        so_deregister_agent_coop_normally();
    }

public:
    demo(context_t ctx, std::string name, so_5::mbox_t  board)
        :   agent_t{std::move(ctx)}
        ,   name_{std::move(name)}
    {
        // Create a subscription for hello message from board.
        so_subscribe(board).event(&demo::on_hello);
    }
};

int main() {
    // Run SObjectizer instance.
    so_5::launch([](so_5::environment_t & env) {
        // Mbox to be used for speading hello message.
        auto board = env.create_mbox();

        // Create several agents in separate coops.
        for(const auto & n : {"Alice"s, "Bob"s, "Mike"s})
            env.register_agent_as_coop(env.make_agent<demo>(n, board));

        // Spread hello message to all subscribers.
        so_5::send<hello>(board, "Hello, World!");
    });
}
