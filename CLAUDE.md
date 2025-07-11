# Claude Development Guidelines for Vibe Keyboard Project

This document provides guidelines for Claude when working on the Vibe Keyboard PlatformIO project.

## Project Overview

This is a PlatformIO-based embedded systems project for developing a vibe keyboard.

## Development Environment

- **Platform**: PlatformIO
- **IDE**: VSCode with PlatformIO extension
- **Version Control**: Git (feature/claude branch)

## Build and Test Commands

### Build
```bash
uv run task build
```

### Upload to Device
```bash
uv run task upload
```

### Clean Build
```bash
uv run task clean
```

### Run Tests
```bash
uv run task test
```

### Serial Monitor
```bash
uv run task monitor
```

### Build and Upload
```bash
uv run task build-upload
```

## Code Style Guidelines

1. **C/C++ Standards**
   - Follow existing code style in the project
   - Use consistent indentation (check existing files)
   - Keep functions focused and small
   - Add meaningful variable and function names

2. **File Organization**
   - Source files in `src/`
   - Header files in `include/`
   - Libraries in `lib/`
   - Tests in `test/`

## Important Files

- `platformio.ini` - Project configuration
- `src/main.cpp` - Main program entry point
- `.gitignore` - Git ignore patterns
- `.claude/settings.local.json` - Claude-specific permissions

## Development Workflow

1. Always check existing code style before making changes
2. Run `uv run task build` to verify code compiles before committing
3. Use meaningful commit messages
4. Create feature branches for new functionality
5. Test on actual hardware when possible

## Security Restrictions

The following operations are restricted in `.claude/settings.local.json`:
- Force push operations
- Container/Kubernetes operations
- Cloud provider CLIs
- System administration commands
- Dangerous filesystem operations

## Common Tasks

### Adding a New Library
```bash
pio pkg install <library-name>
```

### Checking Library Dependencies
```bash
pio pkg list
```

### Board Information
```bash
pio boards
```

## Debugging Tips

1. Use Serial.print() for debugging output
2. Check `pio device monitor` for runtime errors
3. Verify board connections before uploading
4. Use `pio run -v` for verbose build output

## Pre-commit Hooks

This project uses pre-commit hooks that automatically:
- Trim trailing whitespace
- Fix end of files
- Check for large files
- Validate mixed line endings
- Format C/C++ code with clang-format
- Run cpplint checks

## Notes for Claude

- Always verify the build succeeds with `uv run task build` before committing
- Check existing code patterns before implementing new features
- Use the serial monitor to debug runtime issues
- Follow the established project structure
- Remember that this is an embedded system with limited resources
- Use taskipy commands via `uv run task` for all PlatformIO operations
