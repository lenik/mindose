### **Mindose: A Portable, Windows 3.1-like Operating System**

**Mindose** is a **lightweight, portable OS** designed to mimic **Windows 3.1** in terms of its **GUI** and **multi-tasking** capabilities, while using a **Linux-like file system structure**. Unlike Windows, **Mindose** adopts a single root file system (`/`), uses forward slashes (`/`) as the file separator, and supports basic applications, multi-tasking, and memory management. It is compatible with both **x86 (real, protected, long modes)** and **ARM architectures**, and supports the **ELF (.bin)** and **PE (.exe)** executable formats.

Mindose can be executed as a **true OS** or as an **application** within a host operating system, simulating devices using configuration options like `--mem`, `--diskimage`, and `--iso`. When running as an application, Mindose can simulate virtual devices for memory, disk images, and ISO files, making it highly flexible for development and educational purposes.

---

### **Key Features**

1. **Multi-tasking**:

   * **Preemptive multi-tasking** allows multiple applications to run concurrently.
   * Each application has its own process with isolated memory space.

2. **Graphical User Interface (GUI)**:

   * Mindose mimics the **Windows 3.1** GUI, with basic window management, icons, buttons, and menus.
   * Applications like **Notepad**, **Paint**, **Minesweeper**, and **WordPad** are included.

3. **File System**:

   * Mindose uses a **single root file system** (like Linux) with the root directory `/`.
   * File paths use `/` as the separator, aligning with Unix-style file systems.
   * Mindose includes basic file management capabilities like reading, writing, and deleting files.

4. **Device Simulation** (in Application Mode):

   * In **application mode**, Mindose simulates devices through the following options:

     * `--mem SIZE`: Simulate memory with the specified size.
     * `--diskimage FILE`: Simulate a disk device with the provided disk image file.
     * `--iso FILE`: Simulate a CD/DVD device with the specified ISO file.

5. **Supported Architectures**:

   * **x86** (real, protected, long modes) for compatibility with legacy and modern processors.
   * **ARM** for embedded and mobile systems, such as Raspberry Pi or ARM-based tablets.

6. **Executable Formats**:

   * **ELF** (.bin) for Unix-like systems.
   * **PE** (.exe) for Windows applications.

7. **Process Management**:

   * Mindose loads **ELF** and **PE** executable files, running them as processes.
   * A **process manager** handles the lifecycle of running applications, including process scheduling and memory management.

8. **Memory and Disk Management**:

   * Mindose manages **virtual memory** and **disk I/O** with paging and simple file system management.

---

### **System Architecture**

#### **1. Kernel**

The **kernel** handles core operating system tasks such as:

* **Memory management**: Virtual memory, paging, and memory allocation for processes.
* **Task scheduling**: Manages process scheduling with preemptive multi-tasking.
* **Device management**: Handles virtual devices like memory, disk, and CD/DVD drives.

#### **2. Process Manager**

The **process manager** handles the following tasks:

* **Loading Executables**: Loads **ELF** or **PE** binaries into memory.
* **Context Switching**: Performs context switches between processes.
* **System Calls**: Handles system calls for file I/O, memory allocation, and inter-process communication (IPC).

#### **3. GUI and Applications**

The **GUI system** supports window management, basic widgets, and user interaction:

* **Window Management**: Includes support for draggable windows, minimizing, and maximizing.
* **Widgets**: Includes buttons, text boxes, labels, checkboxes, etc.
* **Event Loop**: Manages user input (keyboard, mouse) and dispatches events to appropriate windows or controls.

Basic applications included in **Mindose**:

* **Notepad**: A simple text editor.
* **Paint**: A basic painting application for drawing shapes.
* **Minesweeper**: A classic game of Minesweeper.
* **WordPad**: A simple word processor.

---

### **Command Line Interface (CLI) Options**

#### **1. True OS Mode**

In **True OS mode**, Mindose operates as a complete operating system. It provides a kernel, device drivers, and memory management for running applications.

* **--arch <architecture>**: Specifies the target architecture (e.g., `x86`, `arm`).
* **--mem <size>**: Specifies the amount of memory to allocate for the system (e.g., `--mem 1G`).
* **--diskimage <file>**: Mounts a disk image as a simulated disk device.
* **--iso <file>**: Mounts an ISO file as a virtual CD/DVD device.

#### **2. Application Mode**

In **Application Mode**, Mindose runs as an application within a host operating system, simulating devices like memory, disks, and CDs.

* **--mem <size>**: Simulates a memory device with the specified size.
* **--diskimage <file>**: Simulates a disk device using the provided disk image file.
* **--iso <file>**: Simulates a CD/DVD device using the provided ISO file.

Example command to run Mindose as an application with simulated devices:

```bash
mindose --mem 512M --diskimage mydisk.img --iso myiso.iso
```

---

### **File System Structure**

Mindose uses a **single root file system** similar to Unix-like systems (e.g., Linux). All files are located under `/`, and file paths use `/` as the separator.

* **/bin**: Executable programs.
* **/home**: User directories.
* **/mnt**: Mount point for external devices like disk images or ISO files.
* **/etc**: Configuration files.
* **/dev**: Device files.

---

### **Executable Formats and Architecture Support**

#### **1. Executable Formats**

Mindose supports **ELF** and **PE** executable formats:

* **ELF** (.bin): Used primarily in Unix-like systems, such as Linux and other open-source environments.
* **PE** (.exe): The standard format for Windows executables.

#### **2. Supported Architectures**

Mindose is designed to support multiple processor architectures:

* **x86** (Real Mode, Protected Mode, Long Mode): Supports both legacy and modern x86 processors.
* **ARM**: Supports ARM-based devices (e.g., Raspberry Pi, ARM-based tablets).

---

### **Memory and Process Management**

#### **1. Memory Management**:

* Mindose uses **paging** to manage virtual memory for processes.
* Memory is allocated dynamically, and **disk images** can be used for **swap space** if needed.

#### **2. Process Management**:

* Mindose handles **multi-tasking** by allocating separate memory spaces for each process.
* Each process runs in its own isolated address space, and **context switching** is handled by the OS scheduler.
* Mindose includes a **process manager** to track running processes and manage their lifecycle.

---

### **Device Simulation (Application Mode)**

When running in **Application Mode**, Mindose can simulate various devices using the following options:

* **Memory Simulation** (`--mem <size>`): Simulate a memory device.
* **Disk Simulation** (`--diskimage <file>`): Simulate a disk device using a disk image file.
* **ISO Simulation** (`--iso <file>`): Simulate a CD/DVD device using an ISO file.

Multiple devices can be simulated simultaneously by repeating the options.

---

### **Example Workflow**

#### **1. True OS Mode**

In **True OS mode**, Mindose can boot as a full operating system and load ELF or PE executables. For example:

```bash
mindose --arch x86 --mem 1G --diskimage system.img --iso livecd.iso
```

This command would boot Mindose as a full OS with 1GB of memory, a disk image, and an ISO mounted as a virtual CD.

#### **2. Application Mode**

In **Application Mode**, Mindose can run as an application on a host system, simulating devices for testing or development purposes. For example:

```bash
mindose --mem 512M --diskimage disk.img --iso cd.iso
```

This would start Mindose with 512MB of memory, a disk image, and an ISO mounted as a virtual CD.

---

### **Conclusion**

**Mindose** is a **lightweight, portable OS** that mimics **Windows 3.1** with **multi-tasking**, **graphical applications**, and a **Linux-like file system**. It can run as a full **OS** or as an **application** within another OS, simulating virtual devices such as memory, disk images, and ISOs. With support for both **ELF** and **PE** executable formats, as well as **x86** and **ARM** architectures, Mindose provides a flexible environment for both development and educational purposes, especially in the embedded systems and OS development fields.

