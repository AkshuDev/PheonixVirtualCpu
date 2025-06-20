// Structs file with all the structs

.section .data
.balign 4

// struct vasm_attributes_struct {
// 	uint32_t mode;
// 	uint32_t runtime_mode;
// 	uint16_t security level;
// 	uint16_t entry_behavior;
// 	uint16_t window_type;
// 	uint16_t input_mode;
// 	uint16_t thread_model;
// 	uint16_t network_policy;
// 	uint16_t lifecycle_mode;
// 	uint16_t memory_profile;
// 	uint8_t event_hooks_enabled;
// 	uint16_t debug_mode;

vasm_attributes_struct:
	.word 0x0 // 0 = Let VASM do it, 1 = CLI Application, 2 = GUI Application
	.word 0x0 // 0 = Auto, 1 = Restricted (no host API), 2 = Dynamic Io (Can perform file/IO/network), 3 = Event Driven (Expects Host Events), 4 = Scriptable (Allow during runtime file changes), 5 = Safe Recovery (Will Attempt Auto Repair on Crash), 6 = Interactive Shell, 7 = Stateful VM (Kepps memory across runs / session-persistent), 8 = Low Latency (Minimal Scheduling Latency, 9 = Background Service ( No GUI, TTY, just a background task)
	.hword 0x0 // 0 = Sandboxed, 1 = File System read (host), 2 = Full File System Access (host), 3 = Full Access (Host, basically a normal app)
	.hword 0x1 // 0 = run main func only, 1 = Init System then run main func, 2 = Requires manual start, 3 = preload only no run
	.hword 0x2 // 0 = None, 1 = FullScreen, 2 = Windowed, 3 = Embedded (canvas inside DOM)
	.hword 0x0 // 0 = StdIn, 2 = Input Buffer (PVcpu stdin_buffer), 3 = Async Input Events (host), 3 = GUI Input fields will be used
	.hword 0x1 // 0 = Single, 1 = Multi (Isolated), 2 = Multi (Shared memory), 3 = Fiber Based Concurrency
	.hword 0x0 // 0 = Not Allowed, 1 = OutBound Only, 2 = Full Network, 3 = User Controll Access
	.hword 0x0 // 0 = Unload on exit, 1 = Persistent in memory, 2 = Unloadable manually, 3 = Hot-Reloadable
	.hword 0x0 // 0 = Default Lazy Alloc, 1 = PreAlloc, 2 = Dynamic, 3 = Capped at N MB
	.hword 0x0 // 0 = None, 1 = Startup, 2 = Shutdown, 3 = Full
	.hword 0x0 // 0 = OFF, 1 = Symbols Loaded, 2 = Breakpoint hooks, 3 = Full introspection
// Done
