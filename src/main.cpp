#include <print>

#include "bus.hpp"
#include "cartridge.hpp"


int main(int argc, char* argv[])
{
    std::println("-- A NES emulator by AnGian");

    if (argc < 2) {
        std::println("!! Missing ROM path");
        exit(1);
    }


    Cartridge* cart;
    try {
        cart = new Cartridge(argv[1]);
    } catch (const std::exception& e)  {
        std::println("!! Error loading cartridge: {}", e.what());
        exit(1);
    }
    std::println("Cartridge is valid");
    cart->printDiagnostics();

    auto bus = new Bus();
    bus->insertCartridge(cart);
    bus->reset();

    // SDL init (video, input)

    // Main loop
    bool running = true;
    while (running) {
    //     //handleInput();

    //     // PPU clock is 3x CPU clock
    //     // for (int i=0; i < 3; ++i)
    //     //     bus.ppu.tick();

        bus->cpu()->clock();
    //     // if (bus.ppu.frameReady()) {
    //     //     renderFrame();
    //     // }
    }
}
