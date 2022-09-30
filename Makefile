# nltools/Makefile
#
# This file is part of nltools, part of the Husky fidonet software project
# Use with GNU make v.3.82 or later
# Requires: husky enviroment
#

nltools_LIBS := $(fidoconf_TARGET_BLD) \
		$(smapi_TARGET_BLD) $(huskylib_TARGET_BLD)

nltools_CDEFS := $(CDEFS) \
			-I$(fidoconf_ROOTDIR) \
			-I$(smapi_ROOTDIR) \
			-I$(huskylib_ROOTDIR) \
			-I$(nltools_ROOTDIR)$(nltools_H_DIR)

ifneq ($(USE_HPTZIP), 0)
    ifeq ($(DYNLIBS), 1)
	nltools_LIBZ = -lz
    else
	nltools_LIBZ = -Xlinker -l:libz.a
    endif
    nltools_LIBS += $(hptzip_TARGET_BLD)
    nltools_CFLAGS += -DUSE_HPTZIP
    nltools_CDEFS  += -I$(hptzip_ROOTDIR)
endif

nltools_TARGET	   = nlupd$(_EXE) ulc$(_EXE) nlcrc$(_EXE) nldiff$(_EXE)
nltools_TARGET_OBJ = $(addprefix $(nltools_OBJDIR), $(nltools_TARGET))
nltools_TARGET_BLD = $(addprefix $(nltools_BUILDDIR), $(nltools_TARGET))
nltools_TARGET_DST = $(addprefix $(BINDIR_DST), $(nltools_TARGET))

nldiff_OBJS = $(nltools_OBJDIR)nldiff$(_OBJ) $(nltools_OBJDIR)crc16$(_OBJ)

nlcrc_OBJS  = $(nltools_OBJDIR)crc16$(_OBJ) $(nltools_OBJDIR)nlcrc$(_OBJ)

ulc_OBJS    = $(nltools_OBJDIR)ulcsort$(_OBJ) $(nltools_OBJDIR)trail$(_OBJ) \
	      $(nltools_OBJDIR)ulcomp$(_OBJ) $(nltools_OBJDIR)ulc$(_OBJ) \
	      $(nltools_OBJDIR)string$(_OBJ) $(nltools_OBJDIR)nldate$(_OBJ) \
	      $(nltools_OBJDIR)julian$(_OBJ) $(nltools_OBJDIR)nlfind$(_OBJ)

nlupd_OBJS  = $(nltools_OBJDIR)nlupdate$(_OBJ) $(nltools_OBJDIR)trail$(_OBJ) \
	      $(nltools_OBJDIR)string$(_OBJ) $(nltools_OBJDIR)nldate$(_OBJ) \
	      $(nltools_OBJDIR)julian$(_OBJ) $(nltools_OBJDIR)nlfind$(_OBJ)


ifdef MAN1DIR
    nltools_MAN1PAGES := nlcrc.1 nldiff.1 nlupdate.1 ulc.1
    nltools_MAN1BLD := $(foreach man,$(nltools_MAN1PAGES),$(nltools_BUILDDIR)$(man).gz)
    nltools_MAN1DST := $(foreach man,$(nltools_MAN1PAGES),$(DESTDIR)$(MAN1DIR)$(DIRSEP)$(man).gz)
endif


.PHONY: nltools_build nltools_install nltools_uninstall nltools_clean nltools_distclean \
	nltools_depend nltools_rmdir_DEP nltools_rm_DEPS \
	nltools_clean_OBJ nltools_main_distclean

nltools_build: $(nltools_TARGET_BLD) $(nltools_MAN1BLD)

ifneq ($(MAKECMDGOALS), depend)
 ifneq ($(MAKECMDGOALS), distclean)
  ifneq ($(MAKECMDGOALS), uninstall)
   include $(nltools_DEPS)
  endif
 endif
endif


# Build application
$(nltools_BUILDDIR)nlcrc$(_EXE): $(nlcrc_OBJS) $(nltools_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(nltools_LIBZ)

$(nltools_BUILDDIR)nldiff$(_EXE): $(nldiff_OBJS) $(nltools_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(nltools_LIBZ)

$(nltools_BUILDDIR)ulc$(_EXE): $(ulc_OBJS) $(nltools_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(nltools_LIBZ)

$(nltools_BUILDDIR)nlupd$(_EXE): $(nlupd_OBJS) $(nltools_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(nltools_LIBZ)

# Compile .c files
$(nltools_ALL_OBJS): $(nltools_OBJDIR)%$(_OBJ): $(nltools_SRCDIR)%.c | $(nltools_OBJDIR)
	$(CC) $(nltools_CFLAGS) $(nltools_CDEFS) -o $(nltools_OBJDIR)$*$(_OBJ) $(nltools_SRCDIR)$*.c

$(nltools_OBJDIR): | $(nltools_BUILDDIR) do_not_run_make_as_root
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@


# Build man pages
ifdef MAN1DIR
    $(nltools_MAN1BLD): $(nltools_BUILDDIR)%.gz: $(nltools_MANDIR)% | do_not_run_make_as_root
	gzip -c $(nltools_MANDIR)$* > $(nltools_BUILDDIR)$*.gz
else
    $(nltools_MAN1BLD): ;
endif


# Install
ifneq ($(MAKECMDGOALS), install)
    nltools_install: ;
else
    nltools_install: $(nltools_TARGET_DST) nltools_install_man ;
endif

$(nltools_TARGET_DST): $(nltools_TARGET_BLD) | $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $< $(DESTDIR)$(BINDIR); \
	$(TOUCH) "$@"

ifndef MAN1DIR
    nltools_install_man: ;
else
    nltools_install_man: $(nltools_MAN1DST)

    $(nltools_MAN1DST): $(DESTDIR)$(MAN1DIR)$(DIRSEP)%: $(nltools_BUILDDIR)% | \
	$(DESTDIR)$(MAN1DIR)
	$(INSTALL) $(IMOPT) $(nltools_BUILDDIR)$* $(DESTDIR)$(MAN1DIR); \
	$(TOUCH) "$(DESTDIR)$(MAN1DIR)$(DIRSEP)$*"
endif


# Clean
nltools_clean: nltools_clean_OBJ
	-[ -d "$(nltools_OBJDIR)" ] && $(RMDIR) $(nltools_OBJDIR) || true

nltools_clean_OBJ:
	-$(RM) $(RMOPT) $(nltools_OBJDIR)*

# Distclean
nltools_distclean: nltools_main_distclean nltools_rmdir_DEP
	-[ -d "$(nltools_BUILDDIR)" ] && $(RMDIR) $(nltools_BUILDDIR) || true

nltools_rmdir_DEP: nltools_rm_DEPS
	-[ -d "$(nltools_DEPDIR)" ] && $(RMDIR) $(nltools_DEPDIR) || true

nltools_rm_DEPS:
	-$(RM) $(RMOPT) $(nltools_DEPDIR)*

nltools_main_distclean: nltools_clean
	-$(RM) $(RMOPT) $(nltools_TARGET_BLD)
ifdef MAN1DIR
	-$(RM) $(RMOPT) $(nltools_MAN1BLD)
endif


# Uninstall
nltools_uninstall:
	-$(RM) $(RMOPT) $(nltools_TARGET_DST)
ifdef MAN1DIR
	-$(RM) $(RMOPT) $(nltools_MAN1DST)
endif
