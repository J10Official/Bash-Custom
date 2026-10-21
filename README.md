# Custom POSIX Shell

A C coursework shell using Unix process and filesystem interfaces. It includes custom navigation/listing commands, command history, process inspection, aliases, signal handling, and background-process tracking.

## Build and run

Use Linux or a compatible Unix environment with GCC and Make. Run from the repository root so the existing configuration and log paths resolve as intended.

```bash
make
./shell
```

The [Makefile](Makefile) builds the executable named `shell`. After changing source files, rebuild with `make clean` followed by `make`; the current Makefile does not declare source-file prerequisites on the executable target.

## Explore the implementation

| Area | Files |
|---|---|
| Input and command execution | [main.c](main.c), [command_handler.c](command_handler.c) |
| Navigation and listing | [directory.c](directory.c), [seek.c](seek.c) |
| History and aliases | [log.c](log.c), [alias.c](alias.c), [.myshrc](.myshrc) |
| Processes and signals | [processlist.c](processlist.c), [proclore.c](proclore.c), [signals.c](signals.c), [neonate.c](neonate.c) |
| Network-related functionality | [networking.c](networking.c) |

Try ordinary commands such as `pwd` and `sleep 1`, then inspect the source for the supported syntax of `hop`, `reveal`, `seek`, and the other custom commands. The original behavioral notes below document important differences and limitations.

## Original implementation assumptions

- Erroneous commands are stored in log
- hop .. .. is interpreted as hop .. ; hop .. and similar for others (prev_path updates as such.)
- reveal - and hop - without any previous directory will lead to an error
- entire line colored on reveal -l
- any commands with the substring "log" would not be stored as its implementation is too tedious (allowed in doubts doc)
- the execution time is printed before the prompt for neatness and in case multiple commands finish execution
- When erroneous commands are run in background they would be accepted intially but upon next input error would be thrown for invalid command.
- placing & in front of self implemented commands will run them as usual
- output status of background commands is printed upon taking the next input to avoid situations where user is typing and a message is displayed.
- Virtual memory storage is displayed in bytes
- The alias is logged in the log file
- piping might not work if the command needs no input.
- file > file1 > file2 would not work as it is not handled in a standard way across teminals. file > file1 ; file 1> file2 should be used instead.
- Piplened commands ran sequentially
- Only command stored for activities (sleep instead of sleep 5)
- Default time not assumed for neonate
- the copilot screenshots are compiled in the Ai_usage pdf.
- Max path lengths, max commands and other similar variales are defined as macros and are whatever is mentioned in code.
