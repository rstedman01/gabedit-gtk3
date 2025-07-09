# Gabedit-GTK3 Agents.md File for OpenAI Codex

This Agents.md file provides guidance for OpenAI Codex and other AI agents working in this repository.
---
## Project Overview

**Gabedit** is a graphical user interface (GUI) for computational chemistry packages such as GAMESS, Gaussian, Molcas, and others. The GUI is written in **C** using **GTK2** for the interface and OpenGL for molecular visualization. The goal of this project fork is to **migrate the entire codebase to GTK3, and then to GTK4** while preserving full application functionality.

---

## 🧠 Agent Goals

1. **Modernize the GUI code** by replacing GTK2 functions and macros with their GTK3/4 equivalents while preserving the general structure of the code
2. **Preserve the application's features**, layout, and behavior across versions.
3. **Ensure that code builds cleanly** using `pkg-config` and standard compilation flags for the active GTK version. Additionally ensure the compiler flag `-I/usr/include/gtkX.0` is used where 'X' is the target version of GTK (2/3/4) 

---

## Project Structure For Codex Navigation
`/gl2ps`: OpenGL to PostScript printing library
`/icons`: UI icons
`/pixmap`: Pixmaps for UI elements
`/platforms`: Config files for each platform Gabedit supports
`/po`: Language-related files
`/spglib`: Library for finding and handling crystal symmetries in C
`/src`: Source code, main target for Codex to analyze <br>
|---- `/Common`: Main C source and header files<br>
|---- `/Crystallography`: Source files for crystallography<br>
|---- `/DeMon`: Unsure of functionality, seems to be related to GUI<br>
|---- `/Display`: Source files for the majority of GUI and rendering functionality<br>
|---- `/Files`: Handles file I/O and file selector dialog windows<br>
|---- `/FireFly`: Source files for interfacing with the ab-initio package Firefly <br>
|---- `/Gamess`: Source files for interfacing with GAMESS<br>
|---- `/Gaussian`: Source files for interfacing with Gaussian<br>
|---- `/Geometry`: Source files for dealing with molecular geometry<br>
|---- `/IsotopeDistribution`: Source files for analyzing isotopic distributions<br>
|---- `/Molcas`: Source files for interfacing with MOLCAS<br>
|---- `/MolecularMechanics`: Source files for working with molecular mechanics calculations<br>
|---- `/Molpro`: Source files for interfacing with MOLPRO<br>
|---- `/Mopac`: Source files for interfacing with MOPAC<br>
|---- `/MPQC`: Source files for interfacing with MPQC (Massively Parallel Quantum Chemistry)<br>
|---- `/MultiGrid`: Source files for using the multigrid method to solve differential equations<br>
|---- `/NetWork`: Networking functionality<br>
|---- `/NWChem`: Source files for interfacing with NWChem<br>
|---- `/Orca`: Source files for interfacing with ORCA<br>
|---- `/Psicode`: Source files for interfacing with Psi3/4<br>
|---- `/QChem`: Source files for interfacing with Q-Chem<br>
|---- `/QFF`: Source files for Gabedit's force field implementation<br>
|---- `/SemiEmpirical`: Source files for semi-empirical methods<br>
|---- `/Spectrum`: Spectroscopy-related source files<br>
|---- `/Symmetry`: Files related to molecular symetry<br>
|---- `/Utils`: Source code for utilities related to math and rendering<br>
|---- `/VibrationalCorrections`: Source files related to vibrational frequency calculations, mostly pertains to Gaussian files<br>
`/utils`: Set of folders containing utility scripts and some code for GUI operation<br>

## Coding Instructions for OpenAI Codex
### General
- All code should adhere to the C17 standard
- Retain existing code style, except in cases where major changes are required
- Add brief comments to changes outlining how each change is used
- Replace all deprecated GTK2 constructors and objects with their GTK3 counterparts while keeping overall program logic and function the same
    - Examples of what should be replaced include the following: 
        - `gtk_hbox_new()` -> `gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing)`
        - `gtk_vbox_new()` -> `gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing)`
        - `gtk_table_new()` -> `gtk_grid_new()`
        - `gtk_object_set()` -> `g_object_set()`
        - `gtk_object_get()` -> `g_object_get()`
        - Constants using `GDK_*` -> `GDK_KEY_*`
        - `gtk_main_quit()` -> `g_signal_connect(window,"destroy",...)`
        - `gtk_widget_hide_all()` -> `gtk_widget_set_visible()` (not mandatory, but preferred for compatibility)
        - Direct struct access (i.e. `widget->window`) -> Use getter functions to comply with GTK3 data hiding
### Overview of criteria
- No use of deprecated GTK2-only widgets and/or containers
- No use of direct structure access or `GtkObject` API
- Builds cleanly with `gtk+-3.0` via `pkg-config` provided flags
- Maintains general idea of original GUI layout and function
- Prevent critical GTK runtime warnings from being thrown

## Build Instructions for OpenAI Codex
- For building with GTK3: 
    ```CC=gcc CFLAGS=$(pkg-config --cflags gtk+-3.0) LDFLAGS=$(pkg-config --libs gtk+-3.0) make``` 
