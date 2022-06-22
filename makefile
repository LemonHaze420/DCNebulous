#------------------------------------------------------------------------------
# Nebulaids.mak
#------------------------------------------------------------------------------

PROJECT = Nebulaids

PROJECT_SRC	= \
		main.c\
        dcinit.c\
		sbinit.c\
        tower.c\
        DISPSTR.C\
        TEXCHAR.C\


#mouse support...
#SYSTEM_LIBS = sg_pdms

OPTIMIZE =	0

COMPILER =	SHC
GFXLIB =	KAMUI2
OUTPUT =	ELF

DEBUG  =	TRUE
LIST   =	FALSE

include $(KATANA_ROOT)\Sample\dc.inc
