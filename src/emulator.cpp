#include <print>

#include "cartridge.hpp"
#include "display.hpp"
#include "keyboard.hpp"
#include "bus.hpp"


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

    
    Display* display = new Display();
    auto displayOk = display->initSystemPalette("2C02G_wiki.pal");
    if (!displayOk) {
        std::println("!! Error initializing system palette");
        display->shutdownSdl();
        return 1;
    }
    
    displayOk = display->initSdl();
    if (!displayOk) {
        std::println("!! Error initializing SDL");
        display->shutdownSdl();
        return 1;
    }

    Keyboard* keyboard = new Keyboard();
    
    
    //bus->cpu()->setPC(0xC000); //automation mode
    bus->cpu()->setTracing(true);

    bus->ppu()->fillDummyNameTable();
    bus->ppu()->testNameTables();
    //bus->ppu()->dumpFrameBuffer();

    // Main loop
    bool running = true;
    while (running) {
        // bus->cpu()->clock();

        // //PPU clock is 3x CPU clock
        // for (int i=0; i < 3; ++i)
        //     bus->ppu()->clock();


        if (bus->ppu()->isFrameComplete()) {
            display->render(bus->ppu()->frameBuffer());
            bus->ppu()->clearFrameComplete();
        }

        running = keyboard->handleEvents();
        if (!running)
            std::println("Got SDL quit");
    }

    display->shutdownSdl();
    return 0;
}
