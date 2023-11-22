add_library(ft8 ft8/constants.c ft8/constants.h ft8/crc.c ft8/crc.h ft8/decode.c ft8/decode.h ft8/encode.c ft8/encode.h ft8/ldpc.c ft8/ldpc.h ft8/message.c ft8/text.c ft8/text.h)
target_link_libraries(ft8 pico_multicore)
