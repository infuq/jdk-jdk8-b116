#
# Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 only, as
# published by the Free Software Foundation.  Oracle designates this
# particular file as subject to the "Classpath" exception as provided
# by Oracle in the LICENSE file that accompanied this code.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# You should have received a copy of the GNU General Public License version
# 2 along with this work; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
# or visit www.oracle.com if you need additional information or have any
# questions.
#

# Locate this Makefile
ifeq ($(filter /%, $(lastword $(MAKEFILE_LIST))), )
  makefile_path := $(CURDIR)/$(lastword $(MAKEFILE_LIST))
else
  makefile_path := $(lastword $(MAKEFILE_LIST))
endif
repo_dir := $(patsubst %/makefiles/Makefile, %, $(makefile_path))

# What is the name of this subsystem (langtools, corba, etc)?
subsystem_name := $(notdir $(repo_dir))

# Try to locate top-level makefile
top_level_makefile := $(repo_dir)/../common/makefiles/Makefile
ifneq ($(wildcard $(top_level_makefile)), )
  $(info Will run $(subsystem_name) target on top-level Makefile)
  $(info WARNING: This is a non-recommended way of building!)
  $(info ===================================================)
else
  $(info Cannot locate top-level Makefile. Is this repo not checked out as part of a complete forest?)
  $(error Build from top-level Makefile instead)
endif

all:
	@$(MAKE) -f $(top_level_makefile) $(subsystem_name)
