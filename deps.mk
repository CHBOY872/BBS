file_database.o: file/file_database.c file/file_database.h \
  file/file_structure.h
user_database.o: user/user_database.c user/user_database.h \
  user/user_structure.h
server.o: server/server.c server/../user/user_database.h \
  server/../user/user_structure.h server/../file/file_database.h \
  server/../file/file_structure.h server/server.h
