; ===========================================
; File: fileio.inc
; Purpose: File I/O Flags, Modes, and Syscalls
; Target: Linux AMD64 (x86_64)
; Author: AkshuDev
; ===========================================

; ------------
; File Descriptors
; ------------
%define STDIN        0
%define STDOUT       1
%define STDERR       2

; ------------
; Open Flags (used with sys_open / sys_openat)
; ------------
%define O_RDONLY     0x0000      ; open for reading only
%define O_WRONLY     0x0001      ; open for writing only
%define O_RDWR       0x0002      ; open for reading and writing
%define O_CREAT      0x0040      ; create file if it does not exist
%define O_EXCL       0x0080      ; error if O_CREAT and file exists
%define O_NOCTTY     0x0100      ; do not assign controlling terminal
%define O_TRUNC      0x0200      ; truncate file to zero length
%define O_APPEND     0x0400      ; append on each write
%define O_NONBLOCK   0x0800      ; non-blocking mode
%define O_DSYNC      0x1000      ; write according to synchronized I/O data integrity
%define O_SYNC       0x101000    ; sync file contents and metadata (Linux)
%define O_DIRECTORY  0x20000     ; fail if not a directory
%define O_NOFOLLOW   0x40000     ; do not follow symlinks
%define O_CLOEXEC    0x80000     ; set close-on-exec (FD_CLOEXEC)

; ------------
; File Permission Modes (used with O_CREAT)
; ------------
%define S_IRUSR      0o400       ; user has read permission
%define S_IWUSR      0o200       ; user has write permission
%define S_IXUSR      0o100       ; user has execute permission

%define S_IRGRP      0o040       ; group has read permission
%define S_IWGRP      0o020       ; group has write permission
%define S_IXGRP      0o010       ; group has execute permission

%define S_IROTH      0o004       ; others have read permission
%define S_IWOTH      0o002       ; others have write permission
%define S_IXOTH      0o001       ; others have execute permission

%define S_IRWXU      (S_IRUSR | S_IWUSR | S_IXUSR)
%define S_IRWXG      (S_IRGRP | S_IWGRP | S_IXGRP)
%define S_IRWXO      (S_IROTH | S_IWOTH | S_IXOTH)

; Common mode presets
%define MODE_644     (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)         ; -rw-r--r--
%define MODE_755     (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) ; -rwxr-xr-x
%define MODE_666     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) ; -rw-rw-rw-

; ------------
; openat() Specific Constants
; ------------

%define AT_FDCWD         -100        ; Use current working directory

; Flags used specifically with openat
%define AT_SYMLINK_NOFOLLOW  0x100   ; Do not follow symbolic links
%define AT_REMOVEDIR         0x200   ; Remove directory instead of file with unlinkat
%define AT_EMPTY_PATH        0x1000  ; Allow empty relative pathname (Linux >= 2.6.39)
%define AT_SYMLINK_FOLLOW    0x400   ; Follow symbolic link
%define AT_NO_AUTOMOUNT      0x800   ; Suppress terminal automount traversal

; Additional Linux-specific flags usable with newfstatat, linkat, etc.
%define AT_EACCESS           0x200   ; Check access using effective user/group ID

; ------------
; Syscall Numbers (Linux x86_64)
; ------------
%define SYS_read     0
%define SYS_write    1
%define SYS_open     2           ; Deprecated in x86_64 (use openat)
%define SYS_close    3
%define SYS_stat     4
%define SYS_fstat    5
%define SYS_lstat    6
%define SYS_poll     7
%define SYS_lseek    8
%define SYS_mmap     9
%define SYS_munmap   11
%define SYS_openat   257
%define SYS_newfstatat 262
%define SYS_unlinkat 263
%define SYS_exit     60
%define SYS_exit_group 231
%define SYS_openat 257

; ------------
; Openat Constants
; ------------
%define AT_FDCWD     -100        ; Special value used for openat to indicate "current working dir"