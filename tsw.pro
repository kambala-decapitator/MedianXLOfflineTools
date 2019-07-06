TEMPLATE = subdirs

SUBDIRS = dupescan
dupescan.file = dupescan.pro

equals(QT_MAJOR_VERSION, 4) {
    SUBDIRS += qjson-backport
    dupescan.depends += qjson-backport
}
