LOCALDIR = $${TOPLEVEL}/local
INCLUDEPATH += $${LOCALDIR}/include
# Our plugin assumes it is shipped with libcec in the same path
LIBS += -L$${LOCALDIR}/lib -lcec -ldl
QT += gui-private
