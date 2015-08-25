DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_INC = -I./include/ -I../include/ -I./zky_cut

SRCS = $(wildcard $(DIR_SRC)/*.cpp)
OBJS = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRCS)))
LIB = ../lib/libextor.a
AR = ar
CC = g++
#CFLAGS = -g -Wall 
CFLAGS = -g
all:$(LIB)

#base_share_container.o: ../src/base_share_container.cpp \
#	  ../include/base_share_container.h ../include/ext_common_def.h \
#	  ../../include/UH_Define.h ../../include/utility.h \
#	  ../../include/UH_Define.h ../../include/UT_HashSearch.h \
#	  ../../include/UT_Allocator.h ../../include/UC_Allocator_Recycle.h \
#	  ../../include/UT_HashTable_Pro.h ../../include/UC_ReadConfigFile.h \
#	  ../include/word_IDF_calcor.h ../../include/UC_MD5.h
#	g++ -c -o $@ $^ $(DIR_INC)

$(DIR_OBJ)/%.o:$(DIR_SRC)/%.cpp
	$(CC) -fPIC -c $(CFLAGS) -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $^ $(DIR_INC)

$(LIB):$(OBJS)
	$(AR) -r $@ $^

-include $(OBJS:.o=.d)

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d)

