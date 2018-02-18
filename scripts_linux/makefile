# This file is part of CoIoTeSolver.

# CoIoTeSolver is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# CoIoTeSolver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with CoIoTeSolver. If not, see <http://www.gnu.org/licenses/>.


CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LIBS = -pthread
MKDIR_P = mkdir -p

OUT = ../CoIoTeSolver.out
ODIR = ../obj
SDIR = ../src

_OBJS = main.o \
		coiote_solver_io.o \
		coiote_solver_logic.o \

OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

all: dir $(OUT)

$(OUT): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)


.PHONY: dir
dir: ${ODIR}
${ODIR}:
	${MKDIR_P} ${ODIR}

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(OUT)
	rmdir -p $(ODIR) 2> /dev/null || :
