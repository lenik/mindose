# Mindose OS

A portable, Windows 3.1-like operating system that can run as both a true OS and as an application within a host operating system.

## Features

- **Multi-tasking**: Preemptive multi-tasking with isolated process memory spaces
- **Windows 3.1-like GUI**: Classic window management with draggable windows, buttons, and menus
- **Linux-style File System**: Single root filesystem (`/`) with standard directories
- **Multiple Architecture Support**: x86 (real, protected, long modes) and ARM
- **Executable Format Support**: Both ELF (.bin) and PE (.exe) formats
- **Device Simulation**: Virtual memory, disk images, and ISO mounting in application mode

## Included Applications

Each application is a **real standalone executable** with its own `main()` function and embedded resource section containing icons:

- **Notepad** (`notepad_app`): Simple text editor with New, Open, Save functionality
- **Paint** (`paint_app`): Basic drawing application with brush, line, rectangle, and circle tools  
- **Minesweeper** (`minesweeper_app`): Classic minesweeper game with 10x10 grid

### Application Architecture

- **Standalone Executables**: Each app has its own `main()` function and can run independently
- **Resource Sections**: Applications embed their icons and assets in resource sections
- **Application Loader**: Mindose OS can launch and manage standalone applications as separate processes
- **Icon System**: 16x16 4-bit color icons embedded in each application's resource section

## Building

### Using Meson (Recommended)

```bash
# Setup build directory
meson setup builddir

# Compile the project
meson compile -C builddir

# Or use ninja directly
ninja -C builddir
```

### Using Make (Legacy)

```bash
make all
```

## Running

### Application Mode (Default)

#### With Meson
```bash
# Build the complete system (OS + applications)
meson compile -C builddir

# Run Mindose OS with application launcher
./builddir/mindose --mem 512M

# Run standalone applications directly
./builddir/notepad_app
./builddir/paint_app
./builddir/minesweeper_app

# Test individual applications using Meson run targets
meson compile -C builddir && meson exec -C builddir --no-rebuild test-notepad
meson compile -C builddir && meson exec -C builddir --no-rebuild test-paint
meson compile -C builddir && meson exec -C builddir --no-rebuild test-minesweeper

# Or run directly from build directory
cd builddir && ./notepad_app
cd builddir && ./paint_app
cd builddir && ./minesweeper_app
```

#### With Make (Legacy)
```bash
# Build the complete system (OS + applications)
make all

# Build only applications
make apps

# Run Mindose OS with application launcher
./mindose --mem 512M

# Run standalone applications directly
./notepad_app
./paint_app
./minesweeper_app

# Test individual applications
make test-notepad
make test-paint
make test-minesweeper
```

### Command Line Options

- `--mem SIZE`: Specify memory size (e.g., 256M, 1G)
- `--diskimage FILE`: Mount disk image file as virtual disk
- `--iso FILE`: Mount ISO file as virtual CD/DVD
- `--arch ARCH`: Target architecture (x86, arm)
- `--help`: Show help message

## Architecture

### Core Components

1. **Kernel** (`kernel/`)
   - Memory management with paging
   - Process scheduling and management
   - Device management and simulation

2. **File System** (`fs/`)
   - Unix-like directory structure
   - File operations (create, read, write, delete)
   - Standard directories: `/bin`, `/home`, `/mnt`, `/etc`, `/dev`

3. **GUI System** (`gui/`)
   - Windows 3.1-style interface
   - Window management (minimize, maximize, close)
   - Widget system (buttons, labels, text boxes)
   - Event handling (mouse, keyboard)

4. **Process Manager** (`process/`)
   - ELF and PE executable loading
   - Process creation and termination
   - Context switching

5. **Applications** (`apps/`)
   - Standalone executables with main() functions
   - Embedded resource sections with icons
   - Independent process execution

6. **Resource System** (`resource/`)
   - Icon management and embedding
   - Asset loading and retrieval
   - 16x16 4-bit color icon support

7. **Application Loader** (`process/`)
   - Standalone application execution
   - Process management and lifecycle
   - Application registry and launcher

## File System Structure

```
/
├── bin/        # Executable programs
├── home/       # User directories
├── mnt/        # Mount points for external devices
├── etc/        # Configuration files
└── dev/        # Device files
```

## Development

### Adding New Applications

1. Create header file in `apps/` directory
2. Implement application in corresponding `.c` file
3. Add to `Makefile` APP_SOURCES
4. Include header in `main.c` and add init/cleanup calls

### Extending the GUI

The GUI system supports creating custom widgets:

```c
widget_t* my_widget = gui_create_widget(WIDGET_BUTTON, x, y, width, height);
my_widget->on_click = my_click_handler;
gui_add_child_widget(parent_window, my_widget);
```

## Technical Details

- **Memory Management**: Simple first-fit allocation with block splitting
- **Scheduling**: Round-robin process scheduling
- **Graphics**: Text-mode simulation (80x25 characters)
- **Event System**: Polling-based event handling
- **File I/O**: In-memory file system simulation

## Limitations

This is a demonstration/educational OS with simplified implementations:

- Graphics are simulated in text mode
- No real hardware device drivers
- Simplified memory management
- Basic process isolation
- Limited executable format parsing

## Future Enhancements

- Real graphics mode support
- Network stack implementation
- More sophisticated memory management
- Hardware device drivers
- Additional applications (Calculator, File Manager, etc.)
- Sound system support

## License

Educational/demonstration purposes. See individual source files for specific licensing information.
