# Test makefile

all: test

test: clean test.relative test.absolute test.analyze test.archive

test.relative: mremd.opts ../../CreateRemdDirs
	@echo "----------------------------------------"
	@echo "*** RELATIVE PATH TEST ***"
	@/bin/rm -rf run.000
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 -c ../CRD >> output.txt
	@echo "TEST DIFFS: " 
	@diff mremd.dim.save run.000/remd.dim | wc -l
	@diff relative.groupfile.save run.000/groupfile | wc -l
	@diff in.001.save run.000/INPUT/in.001 | wc -l
	@echo ""

test.absolute: mremd.opts ../../CreateRemdDirs
	@echo "----------------------------------------"
	@echo "*** ABSOLUTE PATH TEST ***"
	@/bin/rm -rf run.000
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 -c ~/bin/createRemdDirs/test/CRD >> output.txt
	@echo "TEST DIFFS: "
	@diff mremd.dim.save run.000/remd.dim | wc -l
	@diff absolute.groupfile.save run.000/groupfile | wc -l
	@diff in.001.save run.000/INPUT/in.001 | wc -l
	@echo ""

run.000/TRAJ: test.absolute

test.analyze: mremd.opts ../../CreateRemdDirs run.000/TRAJ
	@echo "----------------------------------------"
	@echo "*** ANALYZE TEST ***"
	touch run.000/TRAJ/rem.crd.001
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 --analyze >> output.txt
	@echo -n "TEST DIFF: "
	@diff batch.cpptraj.in.save Analyze.0.0/batch.cpptraj.in | wc -l
	@echo ""

test.archive: mremd.opts ../../CreateRemdDirs run.000/TRAJ
	@echo "----------------------------------------"
	@echo "*** ARCHIVE TEST ***"
	touch run.000/TRAJ/rem.crd.001
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 --archive >> output.txt
	@echo "TEST DIFFS: "
	@diff ar1.0.cpptraj.in.save Archive.0.0/ar1.0.cpptraj.in | wc -l
	@diff ar2.0.cpptraj.in.save Archive.0.0/ar2.0.cpptraj.in | wc -l
	@echo ""

test.md.single: md.opts ../../CreateRemdDirs
	@echo "----------------------------------------"
	@echo "*** MD RELATIVE PATH TEST ***"
	@/bin/rm -rf run.000
	$(VALGRIND) ../../CreateRemdDirs -i md.opts -b 0 -e 0 >> output.txt
	@echo "TEST DIFFS: "
	@diff mdsingle.groupfile.save run.000/groupfile | wc -l
	@diff mdsingle.in.save run.000/md.in | wc -l
	@echo ""

test.md.rst: mdrst.opts ../../CreateRemdDirs ConstF.rst
	@echo "----------------------------------------"
	@echo "*** MD W/RST RELATIVE PATH TEST ***"
	@/bin/rm -rf run.000
	$(VALGRIND) ../../CreateRemdDirs -i mdrst.opts -b 0 -e 0 >> output.txt
	@echo "TEST DIFFS: "
	@diff mdsingle.groupfile.save run.000/groupfile | wc -l
	@diff mdrst.in.save run.000/md.in | wc -l
	@echo ""

ConstF.rst:
	@touch ConstF.rst

test.vg:
	$(MAKE) test VALGRIND="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes"

clean:
	/bin/rm -rf run.000 run.001 Analyze.0.0 Archive.0.0 output.txt ConstF.rst
