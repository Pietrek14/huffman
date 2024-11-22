SRC_FILES = main.cpp huffman/huffman.cpp huffman/tree/tree.cpp huffman/message/message.cpp huffman/buffer/buffer.cpp
OBJ_FILES := $(patsubst %.cpp,obj/%.o,$(SRC_FILES))
TARGET_FILE = huffman.exe

build: $(OBJ_FILES)
	g++ $^ -o $(TARGET_FILE)

obj/%.o: %.cpp
	@if not exist "$(subst /,\,$(dir $@))" mkdir $(subst /,\,$(dir $@))
	g++ -c $^ -o $@

run:
	./$(TARGET_FILE)