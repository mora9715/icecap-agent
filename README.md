# Icecap Agent

[![GitHub release](https://img.shields.io/github/v/release/mora9715/icecap-agent)](https://github.com/mora9715/icecap-agent/releases/latest)


Icecap Agent is a **32-bit injectable DLL** for World of Warcraft 3.3.5a that executes commands and publishes events as part of the Icecap ecosystem. The agent runs inside the WoW client process and communicates over a lightweight TCP protocol using Protocol Buffers.

**Compatible with WoW 3.3.5a build 12340 only.**

Main project: [IceCap](https://github.com/mora9715/icecap)

## What it does

The agent is a simple command-event bridge that:

- **Executes commands** received from the controlling system
- **Publishes events** back to the controller

Commands and events are defined in the [icecap-contracts](https://github.com/mora9715/icecap-contracts) repository. When contracts change, this implementation adapts accordingly.

## Key Features

### Core Functionality
- **Embedded TCP server** on port 5050 with robust connection handling
- **Protocol Buffers** messaging for reliable command/event communication
- **Self-unload mechanism** via Delete key with proper edge detection

### Hook System
- **MinHook-based** function hooking for D3D9 EndScene and FrameScript events

## Quick Start

1. **Build the DLL**:
   ```bash
   cmake -B build -A Win32
   cmake --build build --target icecap-agent
   ```

2. **Inject** `injector.dll` into the target WoW client process

3. **Connect** to the agent on port 5050 and send Protocol Buffer commands

4. **Monitor logs** at `%TEMP%\icecap-agent\icecap-agent.log`

5. **Unload** by pressing Delete key in-game

## Documentation

Detailed documentation and usage examples can be found in the main [IceCap repository](https://github.com/mora9715/icecap).

## Disclaimer

This project is intended for **local server environments and educational purposes only**. The software is provided for research and learning. You are solely responsible for how you use it and must ensure compliance with applicable terms of service and laws.
