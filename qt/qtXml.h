#ifndef __MIO_QT_XML_H__
#define __MIO_QT_XML_H__

#include "mio/altro/io.h"

#define INT_TO_QSTR(val) QString().sprintf("%d", val)
#define FLT_TO_QSTR(val) QString().sprintf("%.6f", val)
#define BOOL_TO_QSTR(val) QString().sprintf("%d", val ? 1 : 0)
#define QT_SET_CHECK_STATE(checkBoxPtr, val) checkBoxPtr->setCheckState(val ? Qt::Checked : Qt::Unchecked);
//#define ATTR_TO_QSTR(attr, attr_name) attr.value(attr_name).toString()
#define ATTR_TO_INT(attr, attr_name) attr.value(attr_name).toString().toInt()
#define ATTR_TO_FLT(attr, attr_name) attr.value(attr_name).toString().toFloat()

#define QRefStr2StdStr(ref_str) ref_str.toString().toStdString()

#define ELEM2INT(xml_stream_reader) xml_stream_reader.readElementText().toInt()
#define ELEM2FLT(xml_stream_reader) xml_stream_reader.readElementText().toFloat()

inline void ForceXmlExtension(QString &file_full_qt){
  std::string file_full = file_full_qt.toStdString();
  if( mio::ForceFileExtension(file_full, "xml") )
    file_full_qt = QString::fromStdString(file_full);
}

#endif //__MIO_QT_XML_H__

