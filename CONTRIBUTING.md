# **Contributing to PVCpu**

Thank you for your interest in contributing to **PVCpu (Pheonix Virtual CPU)**!
This project is part of the larger **Pheonix Ecosystem**, and contributions are welcome from anyone who wants to help improve performance, stability, documentation, tooling, or the JIT/virtual architecture.

This document will guide you through the contribution process.

---

## **How to Contribute**

### **1. Fork the Repository**

Click **Fork** on GitHub to create your own copy of the project.

### **2. Clone Your Fork**

```sh
git clone https://github.com/AkshuDev/PheonixVirtualCpu.git
cd PheonixVirtualCpu
```

### **3. Create a Branch**

Always create a new branch for your changes:

```sh
git checkout -b feature/my-new-feature
```

Use descriptive branch names such as:

* `fix/jit-bounds-check`
* `feature/new-loader`
* `docs/isa-update`

---

## **Reporting Issues & Security Vulnerabilities**

### **Bug Reports**

To report a bug:

1. Go to the repository’s **GitHub Issues**
2. Open a new issue using the **Bug Report** template
3. Include logs, steps to reproduce, your OS, and system details if possible

### **Security Issues**

Please follow the process described in the repository’s **SECURITY.md**.

---

## **Pull Request Guidelines**

Before submitting a Pull Request (PR):

### **1. Follow Code Style**

PVCpu uses a clean, strict style for readability and safety.

For **C**:

* Always use consistent indentation (4 spaces preferred)
* Avoid global variables where possible
* Keep functions small and modular
* Document all public functions with comments

For **Pheonix Assembly**:

* Comment every block of logic
* Use meaningful labels/functions
* Follow architecture naming conventions

### **2. Write Clear Commit Messages**

Good example:

```
fix(jit): corrected register spill logic for block merging
```

Bad example:

```
fixed stuff
```

### **3. Keep PRs Focused**

One PR = one change.

Don’t mix documentation, bug fixes, and new features together.

### **4. Add or Update Documentation**

If your change affects the behavior or architecture, update:

* `README.md`
* `ISA.md`
* `ARCHITECTURE.md`
* Any tool documentation

### **5. Add Tests When Possible**

If your change is testable:

* Add tests under `/tests`
* Ensure existing tests still pass

---

## **Development Setup**

### **Requirements**

* C compiler (GCC/Clang/MSVC)
* PAC (For Assembling)
* Make or build scripts depending on your OS

### **Build**

Linux:

```sh
make
```

Windows (MSVC):

```sh
build.bat
```

---

## **Contributing to Tools (PDumper, etc.)**
***NOTE: PAC is a completely seperate project, you cannot contribute to that from here!***

If contributing to the tools inside `/tools`:

* Keep each tool modular
* Do not introduce dependencies that break cross-platform support
* Follow each tool’s individual guidelines (if any)

---

## **Code of Conduct**

By participating in this project, you agree to follow the guidelines of respectful, constructive collaboration.
Harassment, trolling, or discrimination will not be tolerated.

---

## **Need Help?**

If you have questions:

* Open a **Discussion** on GitHub
* Create a **question-type issue**
* Or ask for clarification in your PR
