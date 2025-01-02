# huffman

A simple implementation of Huffman coding.

## Serialization

The encoded message is serialized into a custom format, described below:

**Header section**

|   **Bytes**   |         **Content**       |
| :-----------: | :-----------------------: |
|       3       |            *HFF*          |
|       1       |  version (currently *0*)  |

**Content section**

|   **Size**    |            **Content**           |
| :-----------: | :------------------------------: |
|    2 bytes    |    Huffman tree size (in bits)   |
|    4 bytes    |  encoded message size (in bits)  |
|     n bits    |            tree data             |
|     m bits    |         encoded message          |
|  8k-m-n bits  |             padding              |

**Footer section**

|   **Bytes**   |         **Content**       |
| :-----------: | :-----------------------: |
|       2       |             *XX*          |

**WARNING!** If you're serializing into a string, the string may be terminated by version or content data.

Lengths are stored as little-endian unsigned integers.
