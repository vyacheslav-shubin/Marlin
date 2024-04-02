help:
	@echo "Tasks for local development:"
	@echo "* tests-single-ci:             Run a single test from inside the CI"
	@echo "* tests-single-local:          Run a single test locally"
	@echo "* tests-single-local-docker:   Run a single test locally, using docker-compose"
	@echo "* tests-all-local:             Run all tests locally"
	@echo "* tests-all-local-docker:      Run all tests locally, using docker-compose"
	@echo "* setup-local-docker:          Setup local docker-compose"
	@echo ""
	@echo "Options for testing:"
	@echo "  TEST_TARGET          Set when running tests-single-*, to select the"
	@echo "                       test. If you set it to ALL it will run all "
	@echo "                       tests, but some of them are broken: use "
	@echo "                       tests-all-* instead to run only the ones that "
	@echo "                       run on GitHub CI"
	@echo "  ONLY_TEST            Limit tests to only those that contain this, or"
	@echo "                       the index of the test (1-based)"
	@echo "  VERBOSE_PLATFORMIO   If you want the full PIO output, set any value"
	@echo "  GIT_RESET_HARD       Used by CI: reset all local changes. WARNING:"
	@echo "                       THIS WILL UNDO ANY CHANGES YOU'VE MADE!"
.PHONY: help

tests-single-ci:
	export GIT_RESET_HARD=true
	$(MAKE) tests-single-local TEST_TARGET=$(TEST_TARGET)
.PHONY: tests-single-ci

tests-single-local:
	@if ! test -n "$(TEST_TARGET)" ; then echo "***ERROR*** Set TEST_TARGET=<your-module> or use make tests-all-local" ; return 1; fi
	export PATH=./buildroot/bin/:./buildroot/tests/:${PATH} \
	  && export VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) \
	  && run_tests . $(TEST_TARGET) "$(ONLY_TEST)"
.PHONY: tests-single-local

tests-single-local-docker:
	@if ! test -n "$(TEST_TARGET)" ; then echo "***ERROR*** Set TEST_TARGET=<your-module> or use make tests-all-local-docker" ; return 1; fi
	docker-compose run --rm marlin $(MAKE) tests-single-local TEST_TARGET=$(TEST_TARGET) VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) GIT_RESET_HARD=$(GIT_RESET_HARD) ONLY_TEST="$(ONLY_TEST)"
.PHONY: tests-single-local-docker

tests-all-local:
	export PATH=./buildroot/bin/:./buildroot/tests/:${PATH} \
	  && export VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) \
	  && for TEST_TARGET in $$(./get_test_targets.py) ; do echo "Running tests for $$TEST_TARGET" ; run_tests . $$TEST_TARGET ; done
.PHONY: tests-all-local

tests-all-local-docker:
	docker-compose run --rm marlin $(MAKE) tests-all-local VERBOSE_PLATFORMIO=$(VERBOSE_PLATFORMIO) GIT_RESET_HARD=$(GIT_RESET_HARD)
.PHONY: tests-all-local-docker

setup-local-docker:
	docker-compose build
.PHONY: setup-local-docker


pics: font
	/usr/bin/python3 ./make_res.py

define make_firmware
    mkdir -p .pio/build/$1
    pio run -e $1
    mkdir -p .pio/firmware/$1
    if [ -f .pio/build/$1/Robin_nano35.bin ]; then mv -f .pio/build/$1/Robin_nano35.bin .pio/firmware/$1/Robin_nano35.bin; fi		
    if [ -f .pio/build/$1/Robin_nano_v3.bin ]; then mv -f .pio/build/$1/Robin_nano_v3.bin .pio/firmware/$1/Robin_nano_v3.bin; fi   
    #rm -f .pio/build/$1/firmware.elf
    #rm -f .pio/build/$1/firmware.bin
endef

rn12: gen-date
	$(call make_firmware,RN12)

rn12_303: gen-date
	$(call make_firmware,RN12_303)

rn12-no:
	pio run -e RN12-NO

rn12-no-flash: rn12-no
	st-flash write .pio/build/RN12-NO/firmware.bin 0x08000000

rn12-flash: rn12
	st-flash write .pio/build/RN12/firmware.bin 0x08007000

rn12-bootloader:
	st-flash write misc/rn12-bootloader.bin 0x08000000


rn20:
	$(call make_firmware,RN20)

rn13:
	$(call make_firmware,F4_RN13)

rn30:
	$(call make_firmware,F4_RN30)

f4_bin_v3_usb:
	$(call make_firmware,F4_RN30_USB)


bins: rn12 rn13 rn20 rn30

font:
	/usr/bin/python3 ./font_builder.py
	
pics: font
	make -C ../shui-res pics
	mv -f ../shui-res/RESDUMP.BIN .pio/firmware/
	mv -f ../shui-res/*.PCH .pio/firmware/

font:
	make -C ../shui-res font

shui_wifi:
	make -C ../wifi-module-esp32 release_build
	mv -f ../wifi-module-esp32/SHUIWIFI.BIN .pio/firmware/

shui_blob:
	make -C ../wifi-module-esp32 wifi-blobs

gen-date:
	echo -n "constexpr const char * const CURRENT_DATE=\"">Marlin/src/lcd/extui/lib/shui/date.h
	echo -n `date +%Y-%m-%d`>>Marlin/src/lcd/extui/lib/shui/date.h
	echo "\";">>Marlin/src/lcd/extui/lib/shui/date.h

git-copy-laser:
	cp -fr .pio/firmware/RN12 ../shui/dev/LASER/
	cp -fr .pio/firmware/RN20 ../shui/dev/LASER/
	cp -fr .pio/firmware/F4_RN30 ../shui/dev/LASER/
	cp -fr .pio/firmware/F4_RN13 ../shui/dev/LASER/
	cp -fr .pio/firmware/RESDUMP.BIN ../shui/dev/LASER/RESDUMP.BIN

git-publish-dev:
	make -C ../shui publish-dev

git-publish-laser: git-copy-laser git-publish-dev

copy-ya:
	make -C ../shui-src copy-ya
