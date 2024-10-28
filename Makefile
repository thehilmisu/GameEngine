# Directories
SRCDIR := ./Engine
INCDIR := ./Engine
BINDIR := ./bin
LIBDIR := ./lib
SANDBOXDIR := ./Sandbox

# Files
ENGINE_SRC := $(wildcard $(SRCDIR)/*.cpp)  # All .cpp files in SRCDIR
ENGINE_OBJ := $(ENGINE_SRC:.cpp=.o)        # Corresponding .o files
ENGINE_LIB := $(LIBDIR)/libEngine.a

SANDBOX_SRC := $(wildcard $(SANDBOXDIR)/*.cpp)  # All .cpp files in SANDBOXDIR
SANDBOX_OBJ := $(SANDBOX_SRC:.cpp=.o)           # Corresponding .o files
SANDBOX_BIN := $(BINDIR)/sandbox

# Compiler and Flags
CXX := g++
CXXFLAGS := -I$(INCDIR) -Ithirdparty/spdlog/include -std=c++17
LDFLAGS := -L$(LIBDIR) -lEngine

# Targets
all: $(ENGINE_LIB) $(SANDBOX_BIN)

# Build the engine library
$(ENGINE_LIB): $(ENGINE_OBJ)
	@mkdir -p $(LIBDIR)
	ar rcs $@ $^

# Build sandbox application
$(SANDBOX_BIN): $(SANDBOX_OBJ) $(ENGINE_LIB)
	@mkdir -p $(BINDIR)
	$(CXX) $(SANDBOX_OBJ) -o $@ $(LDFLAGS)

# Compile engine object files
$(ENGINE_OBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile sandbox object files
$(SANDBOX_OBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(BINDIR) $(LIBDIR) $(ENGINE_OBJ) $(SANDBOX_OBJ) $(ENGINE_LIB)

.PHONY: all clean
