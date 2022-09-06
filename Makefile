SRC=main.c
BIN=advgame
BIN_DBG=${BIN}_dbg
APPNAME=Advgame.desktop

WARN=-Wall -pedantic

CFLAGS=${WARN}
LDFLAGS=

CC=cc

${BIN}: ${SRC}
	${CC} ${CFLAGS} ${SRC} ${LDFLAGS} -o $@

debug: ${SRC}
	${CC} -g ${CFLAGS} ${SRC} ${LDFLAGS} -o ${BIN_DBG}

.PHONY:
clean:
	${RM} *.o ${RM} ${BIN} ${RM} ${BIN_DBG} ${RM} ${APPNAME}

.PHONY:
install: ${BIN}
	echo "[Desktop Entry]" > ${APPNAME}
	echo "Version=1.0" >> ${APPNAME}
	echo "Type=Application" >> ${APPNAME}
	echo "Name=Advgame" >> ${APPNAME}
	echo "Icon=utilities-terminal" >> ${APPNAME}
	echo "Terminal=true" >> ${APPNAME}
	echo "StartupNotify=false" >> ${APPNAME}
	echo "Exec=${PWD}/${BIN}" >> ${APPNAME}
	echo "Path=${PWD}" >> ${APPNAME}
	echo "GenericName=Adventure Game" >> ${APPNAME}
	chmod 755 ${APPNAME}
	cp ${APPNAME} ~/.local/share/applications/

.PHONY:
uninstall:
	${RM} ~/.local/share/applications/${APPNAME}
