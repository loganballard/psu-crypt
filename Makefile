CC=g++

psucrypt: psucrypt.cpp helpers.cpp
	    $(CC) -o psucrypt psucrypt.cpp helpers.cpp
