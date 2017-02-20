#include <QXmlStreamWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include "mio/altro/io.h"
#include "mio/altro/types.h"


class SvgPrimitive{
  public:
    float stroke_width;
    color3ub_t stroke, fill;
    QString id;

    void SetId(const size_t id_){
      id = QString("primitive_") + QString::number(id_);
    }

    void virtual Write(QXmlStreamWriter &xml_writer) = 0;

    void SetCommon(const float &stroke_width_, const color3ub_t &stroke_, const color3ub_t &fill_, const QString &id_){
      stroke_width = stroke_width_;
      stroke = stroke_;
      fill = fill_;
      id = id_;
    }

  protected:
    void WriteCommon(QXmlStreamWriter &xml_writer){
      xml_writer.writeAttribute( "stroke-width", QString().sprintf("%.7f", stroke_width) );
      xml_writer.writeAttribute( "stroke", QString().sprintf("#%02X%02X%02X", stroke.r, stroke.g, stroke.b) );
      xml_writer.writeAttribute( "fill", QString().sprintf("#%02X%02X%02X", fill.r, fill.g, fill.b) );
      xml_writer.writeAttribute("id", id);
    }
};


struct SvgGroup{
  size2f_t size;
  vertex2f_t translate;
  QString id;
  std::vector<SvgPrimitive*> primitive_vec;
  
  void ClearPrimVec(){
    for(auto &prim : primitive_vec)
      delete prim;
  }

  void Write(QXmlStreamWriter &xml_writer){
    xml_writer.writeStartElement("g");
    xml_writer.writeAttribute( "transform", QString().sprintf("translate(%.5f,%.5f)", translate.x, translate.y) );
    xml_writer.writeAttribute("id", id);
    for(auto &prim : primitive_vec)
      prim->Write(xml_writer);
    xml_writer.writeEndElement(); //end tag g
  }
};


class SvgCircle : public SvgPrimitive{
  public:
    vertex2f_t center;
    float radius;

    SvgCircle(vertex2f_t center_, float radius_) : center(center_), radius(radius_) {}

    SvgCircle(float stroke_width_, color3ub_t stroke_, color3ub_t fill_, QString id_,
              vertex2f_t center_, float radius_) :
              center(center_), radius(radius_) {
      SetCommon(stroke_width_, stroke_, fill_, id_);
    }

    void Write(QXmlStreamWriter &xml_writer){
      xml_writer.writeStartElement("circle");
      WriteCommon(xml_writer);
      xml_writer.writeAttribute( "cx", QString().sprintf("%.7f", center.x) );
      xml_writer.writeAttribute( "cy", QString().sprintf("%.7f", center.y) );
      xml_writer.writeAttribute( "r", QString().sprintf("%.7f", radius) );
      xml_writer.writeEndElement(); //end tag circle
    }
};

class SvgEllipse : public SvgPrimitive{
  public:
    vertex2f_t center, radii;

    SvgEllipse(vertex2f_t center_, vertex2f_t radii_) : center(center_), radii(radii_) {}

    SvgEllipse(float stroke_width_, color3ub_t stroke_, color3ub_t fill_, QString id_,
               vertex2f_t center_, vertex2f_t radii_) :
               center(center_), radii(radii_) {
      SetCommon(stroke_width_, stroke_, fill_, id_);
    }

    void Write(QXmlStreamWriter &xml_writer){
      xml_writer.writeStartElement("ellipse");
      WriteCommon(xml_writer);
      xml_writer.writeAttribute( "cx", QString().sprintf("%.7f", center.x) );
      xml_writer.writeAttribute( "cy", QString().sprintf("%.7f", center.y) );
      xml_writer.writeAttribute( "rx", QString().sprintf("%.7f", radii.x) );
      xml_writer.writeAttribute( "ry", QString().sprintf("%.7f", radii.y) );      
      xml_writer.writeEndElement(); //end tag ellipse
    }
};

class SvgRect : public SvgPrimitive{
  public:
    vertex2f_t origin;
    size2f_t size;

    SvgRect(vertex2f_t origin_, size2f_t size_) : origin(origin_), size(size_) {}

    SvgRect(float stroke_width_, color3ub_t stroke_, color3ub_t fill_, QString id_,
            vertex2f_t origin_, size2f_t size_) :
            origin(origin_), size(size_) {
      SetCommon(stroke_width_, stroke_, fill_, id_);
    }

    void Write(QXmlStreamWriter &xml_writer){
      xml_writer.writeStartElement("rect");
      WriteCommon(xml_writer);
      xml_writer.writeAttribute( "x", QString().sprintf("%.7f", origin.x) );
      xml_writer.writeAttribute( "y", QString().sprintf("%.7f", origin.y) );
      xml_writer.writeAttribute( "height", QString().sprintf("%.7f", size.height) );
      xml_writer.writeAttribute( "width", QString().sprintf("%.7f", size.width) );   
      xml_writer.writeEndElement(); //end tag rect
    }
};


class SvgSimpleText : public SvgPrimitive{
  public:
    QString text;
    float font_size;
    vertex2f_t origin;
    
    SvgSimpleText(const QString text_, const float font_size_, const vertex2f_t origin_, QString id_) :
                  text(text_), font_size(font_size_), origin(origin_){
      SetCommon(0, color3ub_t(0,0,0), color3ub_t(0,0,0), id_);
    }

    void Write(QXmlStreamWriter &xml_writer){
      xml_writer.writeStartElement("text");
      xml_writer.writeAttribute("xml:space", "preserve");
      xml_writer.writeAttribute("text-anchor", "start"); //start, middle, end
      xml_writer.writeAttribute("font-family", "serif");
      xml_writer.writeAttribute( "font-size", QString().sprintf("%.7f", font_size) );
      xml_writer.writeAttribute( "y", QString().sprintf("%.7f", origin.y) );
      xml_writer.writeAttribute( "x", QString().sprintf("%.7f", origin.x) );
      WriteCommon(xml_writer);
      xml_writer.writeCharacters(text);
      xml_writer.writeEndElement(); //end tag text
    }
};


void SaveSvg(std::string file_full_, SvgGroup &svg_grp, size2f_t doc_size){
  EXP_CHK(!file_full_.empty(), return)
  mio::ForceFileExtension(file_full_, "svg");

  QString file_full = QString::fromStdString(file_full_);
  QFile file(file_full);
  EXP_CHK_M(file.open(QIODevice::WriteOnly), return, "File is read only. Need permission to write.")

  QXmlStreamWriter xml_writer;
  xml_writer.setDevice(&file); //set device (here file)to streamwriter
  xml_writer.writeStartDocument(); //write XML version number
  xml_writer.writeStartElement("svg");

  //write SVG "header"
  xml_writer.writeAttribute("xmlns", "http://www.w3.org/2000/svg");
  xml_writer.writeAttribute("xmlns:svg", "http://www.w3.org/2000/svg");
  xml_writer.writeAttribute("version", "1.1");
  xml_writer.writeAttribute( "height", QString().sprintf("%.2fmm", doc_size.height) );
  xml_writer.writeAttribute( "width", QString().sprintf("%.2fmm", doc_size.width) );
  xml_writer.writeAttribute( "viewBox", QString().sprintf("0 0 %.2f %.2f", doc_size.width, doc_size.height) );

  svg_grp.Write(xml_writer);

  xml_writer.writeEndElement(); //end tag svg
  xml_writer.writeEndDocument(); //end document
}

