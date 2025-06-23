include $(APPDIR)/Make.defs

# Hello, World! built-in application info

PROGNAME  = $(CONFIG_EXAMPLES_HELLO_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_HELLO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_HELLO_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_HELLO)

# Hello, World! Example

MAINSRC = app_main.c

include $(APPDIR)/Application.mk
