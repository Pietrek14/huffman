SRC_FILES = src/main.cpp src/interface.cpp src/huffman/huffman.cpp src/huffman/tree/tree.cpp src/huffman/message/message.cpp src/huffman/buffer/buffer.cpp
OBJ_FILES := $(patsubst src/%.cpp,obj/%.o,$(SRC_FILES))
TARGET_FILE = hff.exe

build: $(OBJ_FILES)
	g++ $^ -o $(TARGET_FILE)

obj/%.o: src/%.cpp
	@if not exist "$(subst /,\,$(dir $@))" mkdir $(subst /,\,$(dir $@))
	g++ -c $^ -o $@

run:
	./$(TARGET_FILE)