
CC=occ
ODIR=o
_MD_OBJ=md4c.a
_BABEL_OBJ=init.a dispatch.a import.a importer.a translate.a opts.a export.a exporter.a
_IMPOPT_OBJ= importOpt.a opts.a
_EXPOPT_OBJ= exportOpt.a opts.a
_FILTER_OBJ=filter.a 

MD_OBJ=$(patsubst %,$(ODIR)/%,$(_MD_OBJ))
BABEL_OBJ=$(patsubst %,$(ODIR)/%,$(_BABEL_OBJ))
IMPOPT_OBJ=$(patsubst %,$(ODIR)/%,$(_IMPOPT_OBJ))
EXPOPT_OBJ=$(patsubst %,$(ODIR)/%,$(_EXPOPT_OBJ))
FILTER_OBJ=$(patsubst %,$(ODIR)/%,$(_FILTER_OBJ))

DEPS=

all: MarkdownGS $(ODIR)/._MarkdownGS.r

MarkdownGS: $(ODIR)/md $(ODIR)/babel $(ODIR)/importOpt $(ODIR)/filter $(ODIR)/exportOpt
	@touch MarkdownGS
	@iix chtyp -t 0x00BE -a 0x4003 $@


$(ODIR)/md4c.a: md4c.c md4c.h

$(ODIR)/init.a: init.c

$(ODIR)/dispatch.a: dispatch.c dispatch.h import.h

$(ODIR)/import.a: import.c import.h dispatch.h translate.h opts.h defs.h importer.h

$(ODIR)/translate.a: translate.c translate.h defs.h opts.h

$(ODIR)/importer.a: importer.c importer.h dispatch.h translate.h md4c.h defs.h

$(ODIR)/opts.a: opts.c opts.h

$(ODIR)/filter.a: filter.c

$(ODIR)/importOpt.a: importOpt.c opts.h

$(ODIR)/export.a: export.c export.h exporter.h opts.h

$(ODIR)/exporter.a: exporter.c exporter.h defs.h opts.h export.h

$(ODIR)/exportOpt.a: exportOpt.c opts.h

$(ODIR)/md: $(MD_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(MD_OBJ)) keep=$(basename $@)
	cp $@ MarkdownGS

$(ODIR)/babel: $(BABEL_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(BABEL_OBJ)) keep=$(basename $@)

$(ODIR)/importOpt: $(IMPOPT_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(IMPOPT_OBJ)) keep=$(basename $@)

$(ODIR)/filter: $(FILTER_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(FILTER_OBJ)) keep=$(basename $@)

$(ODIR)/exportOpt: $(EXPOPT_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(EXPOPT_OBJ)) keep=$(basename $@)

$(ODIR)/%.a: %.c $(DEPS)
	@mkdir -p o
	$(CC) --lint=-1 -O -1 -c -o $@ $<

$(ODIR)/._MarkdownGS.r:  md.rez md.equ $(ODIR)/babel $(ODIR)/filter $(ODIR)/importOpt $(ODIR)/exportOpt 
	@mkdir -p o
	$(CC) -o $(ODIR)/MarkdownGS.r md.rez
	cp $@ ._MarkdownGS

clean:
	@rm -f $(ODIR)/*.a $(ODIR)/*.root $(ODIR)/md 
	@rm -f $(ODIR)/babel $(ODIR)/filter $(ODIR)/importOpt $(ODIR)/exportOpt
	@rm -f $(ODIR)/*.r $(ODIR)/._*

