#ifndef __QWT_GRAPH_H__
#define __QWT_GRAPH_H__

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_curve_fitter.h>
#ifdef WITH_QWT_SPLINE
#include <qwt_spline_curve_fitter.h>
#include <qwt_spline_local.h>
#include <qwt_spline_cubic.h>
#include <qwt_spline_pleasing.h>
#include <qwt_spline_parametrization.h>
#endif

#include <QPrinter>
#include <QPrintDialog>
#include <QVBoxLayout>
#include <QWidget>

#include "mio/altro/error.h"
#ifdef HAVE_ADV_SLIDER_WIDGET
#include "mio/qt/advSliderWidget/advSliderWidget.h"
#endif


#ifdef WITH_QWT_SPLINE
class MySplineFitter: public QwtCurveFitter{
  QwtSpline *m_qwt_spline;

  void setBoundaryConditions(int condition, double value = 0.0){
    // always the same at both ends
    m_qwt_spline->setBoundaryCondition(QwtSpline::AtBeginning, condition);
    m_qwt_spline->setBoundaryValue(QwtSpline::AtBeginning, value);
    m_qwt_spline->setBoundaryCondition(QwtSpline::AtEnd, condition);
    m_qwt_spline->setBoundaryValue(QwtSpline::AtEnd, value);
  }

  public:
    enum Mode{
      PChipSpline,
      AkimaSpline,
      CubicSpline,
      CardinalSpline,
      ParabolicBlendingSpline,
      PleasingSpline
    };

    MySplineFitter(Mode mode = CubicSpline) : QwtCurveFitter(QwtCurveFitter::Path){
      switch(mode){   
        case PleasingSpline:
          {
          m_qwt_spline = new QwtSplinePleasing();
          break;
          }
        case PChipSpline:
          {   
          m_qwt_spline = new QwtSplineLocal(QwtSplineLocal::PChip);
          break;
          }
        case AkimaSpline:
          {   
          m_qwt_spline = new QwtSplineLocal(QwtSplineLocal::Akima);
          break;
          }
        case CubicSpline:
          {   
          m_qwt_spline = new QwtSplineCubic();
          break;
          }
        case CardinalSpline:
          {   
          m_qwt_spline = new QwtSplineLocal(QwtSplineLocal::Cardinal);
          break;
          }
        case ParabolicBlendingSpline:
          {   
          m_qwt_spline = new QwtSplineLocal(QwtSplineLocal::ParabolicBlending);
          break;
          }
      }
    }

    ~MySplineFitter(){
      delete m_qwt_spline;
    }

    void setClosing(bool on){
      m_qwt_spline->setBoundaryType(on ? QwtSpline::ClosedPolygon : QwtSpline::ConditionalBoundaries);
    }

    void setBoundaryCondition(const QString &condition = "Natural"){
      QwtSplineC2 *splineC2 = dynamic_cast<QwtSplineC2 *>(m_qwt_spline);
      if(splineC2){
        if(condition == "Cubic Runout"){
          setBoundaryConditions(QwtSplineC2::CubicRunout);
          return;
        }

        if(condition == "Not a Knot"){
          setBoundaryConditions(QwtSplineC2::NotAKnot);
          return;
        }
      }

      QwtSplineC1 *splineC1 = dynamic_cast<QwtSplineC1 *>(m_qwt_spline);
      if(splineC1){
        if(condition == "Linear Runout"){
          setBoundaryConditions(QwtSpline::LinearRunout, 0.0);
          return;
        }

        if(condition == "Parabolic Runout"){
          // Parabolic Runout means clamping the 3rd derivative to 0.0
          setBoundaryConditions(QwtSpline::Clamped3, 0.0);
          return;
        }
      }

      // Natural
      setBoundaryConditions(QwtSplineC1::Clamped2, 0.0);
    }

    void setParametric(const QString &parameterType = "Chordal"){
      QwtSplineParametrization::Type type = QwtSplineParametrization::ParameterX;
      if(parameterType == "Uniform")
        type = QwtSplineParametrization::ParameterUniform;
      else if(parameterType == "Centripetral")
        type = QwtSplineParametrization::ParameterCentripetal;
      else if(parameterType == "Chordal")
        type = QwtSplineParametrization::ParameterChordal;
      else if(parameterType == "Manhattan")
        type = QwtSplineParametrization::ParameterManhattan;

      m_qwt_spline->setParametrization(type);
    }

    virtual QPolygonF fitCurve( const QPolygonF &points ) const{
      const QPainterPath path = m_qwt_spline->painterPath( points );
      const QList<QPolygonF> subPaths = path.toSubpathPolygons();
      if(subPaths.size() == 1)
        subPaths.first();

      return QPolygonF();
    }

    virtual QPainterPath fitCurvePath( const QPolygonF &points ) const{
      return m_qwt_spline->painterPath(points);
    }
};
#endif


class MyGraph : public QObject{
  Q_OBJECT

  private:
    std::vector<QwtPlotCurve*> qwt_plot_curve_vec;
#ifdef WITH_QWT_SPLINE
    std::vector<MySplineFitter*> spline_fitter_vec;
#endif
    int m_last_set_curve_data_flags;
    bool magnify_, pan_;

  public:
    QwtPlot *qwt_plot;
    QwtLegend *qwt_legend;
    QwtPlotGrid *qwt_plot_grid;
    QwtPlotMagnifier *qwt_plot_mag;
    QwtPlotPanner *qwt_plot_pan;
    QwtSymbol *symbol_;
    std::vector<int> qt_global_colors;

    enum {
      FIT_SPLINE = 1,
      PLOT_DOTS = 2,
      DEFAULT_RAND_COLOR = 4,
      COPY_CURVE_DATA = 8,
      ATTACH_CURVE_TO_PLOT = 16,
      MARKERS = 32
    };

    MyGraph(const bool magnify = true, const bool pan = false, const bool with_window_controls = true) :
        m_last_set_curve_data_flags(-1), magnify_(magnify), pan_(pan) {
      qt_global_colors = {2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
      qwt_plot = new QwtPlot;
      qwt_legend = new QwtLegend;
      qwt_plot_grid = new QwtPlotGrid;
      symbol_ = new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );

      if(!with_window_controls){
        Qt::WindowFlags flags = qwt_plot->windowFlags();
        flags |= Qt::CustomizeWindowHint;
        flags &= ~Qt::WindowCloseButtonHint;
        flags &= ~Qt::WindowMinimizeButtonHint;
        flags &= ~Qt::WindowMaximizeButtonHint;
        //flags |= Qt::Tool;
        qwt_plot->setWindowFlags(flags);
      }

      qwt_plot->insertLegend(qwt_legend);
      qwt_plot_grid->enableXMin(true);
      qwt_plot_grid->enableYMin(true);
      qwt_plot_grid->setMinorPen(Qt::gray, 0.0, Qt::DotLine);
      qwt_plot_grid->attach(qwt_plot);
      if(magnify_)
        qwt_plot_mag = new QwtPlotMagnifier( qwt_plot->canvas() );
      if(pan_)
        qwt_plot_pan = new QwtPlotPanner( qwt_plot->canvas() );


      qwt_plot->setCanvasBackground(Qt::white);

      connect(this, SIGNAL(ExternalReplotSignal()), this->qwt_plot, SLOT(replot()), Qt::QueuedConnection);
    }

    ~MyGraph(){
      disconnect(this, SIGNAL(ExternalReplotSignal()), this->qwt_plot, SLOT(replot())); 
      ClearPlot(true);
      DeleteCurves();
      qwt_plot_grid->detach();
      delete qwt_plot_grid;
      delete qwt_legend;
      if(magnify_)
        delete qwt_plot_mag;
      if(pan_)
        delete qwt_plot_pan;
      delete qwt_plot;
    }

    void SetPlotSize(const size_t width, const size_t height){
      if(width > 25 && height > 25)
        qwt_plot->resize(width, height);
    }

    // Detaches all the curves from the plot
    void ClearPlot(const bool replot = false){
      for(size_t i = 0; i < qwt_plot_curve_vec.size(); ++i)
        qwt_plot_curve_vec[i]->detach();
      if(replot)
        qwt_plot->replot();
    }

    // Detaches all the curves from the plot and sets the number of curves to zero
    void DeleteCurves(){
      ClearPlot();
      for(size_t i = 0; i < qwt_plot_curve_vec.size(); ++i){
        delete qwt_plot_curve_vec[i];
#ifdef WITH_QWT_SPLINE
        delete spline_fitter_vec[i];
#endif
      }
      qwt_plot_curve_vec.clear();
#ifdef WITH_QWT_SPLINE
      spline_fitter_vec.clear();
#endif
      m_last_set_curve_data_flags = -1;
    }

    // Set the number of curves allocated
    void SetNumCurve(const size_t size, const bool attach_plots = true){
      STD_INVALID_ARG_E(size > 0 && size < 1000)
      DeleteCurves();
      qwt_plot_curve_vec.resize(size);
#ifdef WITH_QWT_SPLINE
      spline_fitter_vec.resize(size);
#endif
      for(size_t i = 0; i < qwt_plot_curve_vec.size(); ++i){
        qwt_plot_curve_vec[i] = new QwtPlotCurve();
#ifdef WITH_QWT_SPLINE
        spline_fitter_vec[i] = new MySplineFitter();
#endif
        if(attach_plots)
          qwt_plot_curve_vec[i]->attach(qwt_plot);
      }
    }

    // Returns the number of curves currently allocated
    size_t GetNumCurve(){
      return qwt_plot_curve_vec.size();
    }

    // Sets the data of the curve at index 'curve_idx' and attaches it to the plot
    void SetCurveData(const size_t curve_idx, const double *x, const double *y,
                      const size_t size, const int flags = DEFAULT_RAND_COLOR | COPY_CURVE_DATA){
      STD_INVALID_ARG_E( curve_idx >= 0 && curve_idx < qwt_plot_curve_vec.size() )
      QwtPlotCurve *qwt_plot_curve = qwt_plot_curve_vec[curve_idx];
      qwt_plot_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
      if(flags & MARKERS)
        qwt_plot_curve->setSymbol(symbol_);

      //if(m_last_set_curve_data_flags != flags){
        if(flags & FIT_SPLINE){
#ifdef WITH_QWT_SPLINE
          qwt_plot_curve->setStyle(QwtPlotCurve::Lines);
          qwt_plot_curve->setCurveFitter(spline_fitter_vec[curve_idx]);
          qwt_plot_curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
#endif
        }
        else{
          qwt_plot_curve->setCurveAttribute(QwtPlotCurve::Fitted, false);
          if(flags & PLOT_DOTS)
            qwt_plot_curve->setStyle(QwtPlotCurve::Dots);
        }
        if(flags & DEFAULT_RAND_COLOR)
          SetPen(curve_idx, curve_idx, 2);
      //}

      if(flags & COPY_CURVE_DATA)
        qwt_plot_curve_vec[curve_idx]->setSamples(x, y, size); //this makes a deep copy of the data
      else
        qwt_plot_curve_vec[curve_idx]->setRawSamples(x, y, size);
      //normally, the curves were attached in SetNumCurve() and this doesn't need to be done
      if(flags & ATTACH_CURVE_TO_PLOT)
        qwt_plot_curve->attach(qwt_plot);

      m_last_set_curve_data_flags = flags;
    }

    void SetPen(const size_t curve_idx, int color_idx = -1, const size_t pen_width = 1){
      STD_INVALID_ARG_E( curve_idx >= 0 && curve_idx < qwt_plot_curve_vec.size() )
      const size_t num_colors = qt_global_colors.size();
      if(color_idx < 0)
        color_idx = 0;
      qwt_plot_curve_vec[curve_idx]->setPen(static_cast<Qt::GlobalColor>( qt_global_colors[color_idx%num_colors] ), pen_width);
    }

    //QColor(int r, int g, int b, int a = 255)
    void SetPen(const size_t curve_idx, const QColor &color, qreal width = 0.0, Qt::PenStyle style = Qt::SolidLine){
      STD_INVALID_ARG_E( curve_idx >= 0 && curve_idx < qwt_plot_curve_vec.size() )
      qwt_plot_curve_vec[curve_idx]->setPen(color, width, style);
    }

    void SetCurveTitle(const size_t curve_idx, const std::string &title){
      STD_INVALID_ARG_E( curve_idx >= 0 && curve_idx < qwt_plot_curve_vec.size() )
      qwt_plot_curve_vec[curve_idx]->setTitle( QString::fromStdString(title) );
    }

    void PrintPlot(const std::string &file_full, const bool show_dialog = true, const bool in_color = true){
      QPrinter printer(QPrinter::HighResolution);
      printer.setOrientation(QPrinter::Landscape);
      printer.setOutputFileName( QString::fromStdString(file_full) );
      printer.setColorMode(in_color ? QPrinter::Color : QPrinter::GrayScale);

      if(show_dialog){
        QPrintDialog dialog(&printer);
        EXP_CHK(dialog.exec(), return)
      }

      QwtPlotRenderer renderer;
      if(printer.colorMode() == QPrinter::GrayScale){
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame);
        renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
      }
      renderer.renderTo(qwt_plot, printer);
    }

    void SavePlot(const std::string &file_full){
      QwtPlotRenderer renderer;
      renderer.renderDocument(qwt_plot, QString::fromStdString(file_full), QSizeF(100, 100), 85);
    }
/*
   void PrintPlot(QwtPlot &qwt_plot, const std::string file_full){
     QPrinter printer(QPrinter::HighResolution);

     QString docName = qwt_plot->title().text();
     if( !docName.isEmpty() ){
       //docName.replace( QRegExp( QString::fromLatin1("\n") ), tr(" -- ") );
       printer.setDocName(docName);
     }

     //printer.setDocName( QString( file_full.c_str() ) );
     //printer.setCreator("Bode example");
     printer.setOrientation(QPrinter::Landscape);

     QPrintDialog dialog(&printer);
     if( dialog.exec() ){
       QwtPlotRenderer renderer;

       if(printer.colorMode() == QPrinter::GrayScale){
         renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
         renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
         renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame);
         renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
       }

       renderer.renderTo(&qwt_plot, printer);
     }
   }
*/
    void ExternalReplot(){
      emit ExternalReplotSignal();
    }

  public slots:
    void UpdateLayout(){
      qwt_plot->updateLayout();
      qwt_plot->updateCanvasMargins();
      qwt_plot->updateAxes();
    }

  signals:
    void ExternalReplotSignal();

};

typedef MyGraph Graph;


#ifdef HAVE_ADV_SLIDER_WIDGET
class CMySliderGraph : public QWidget{
  Q_OBJECT

  std::vector<CDoubleAdvSliderWidget*> slider_vec;
  QVBoxLayout *vert_layout;
  bool ui_is_init;

  public:
    MyGraph *graph;

    CMySliderGraph() : ui_is_init(false){
      graph = new MyGraph;
      slider_vec.clear();
    }

    ~CMySliderGraph(){
      for(auto &slider : slider_vec){
        disconnect( slider, SIGNAL( ValueChanged(double) ), this, SLOT( Replot() ) );
        delete slider;
      }
      delete vert_layout;
      delete graph;
    }

    void InitUI(){
      STD_INVALID_ARG_EM(!ui_is_init, "you called InitUI() twice")
      vert_layout = new QVBoxLayout();
      vert_layout->addWidget(graph->qwt_plot);
      for(auto &slider : slider_vec){
        vert_layout->addWidget(slider);
        connect( slider, SIGNAL( ValueChanged(double) ), this, SLOT( Replot() ) );
      }

      setLayout(vert_layout); //member function of QWidget, here it is peformed on centralWidget
      ui_is_init = true;
      Replot();
      setMinimumSize(800, 600);
    }

    void SetNumSlider(const size_t num_slider){//, const size_t num_dec = 3, const double abs_min = -999, const double abs_max = 999){
      STD_INVALID_ARG_EM(!ui_is_init, "you must call SetNumSlider() before InitUI() is called")
      STD_INVALID_ARG_EM(slider_vec.size() == 0, "you called SetNumSlider() twice")
      slider_vec.resize(num_slider);
      //for(auto &slider : slider_vec)
        //slider = new CDoubleAdvSliderWidget(abs_min, abs_max, 0.0, num_dec, abs_min, abs_max);
      for(auto &slider : slider_vec)
        slider = new CDoubleAdvSliderWidget();
    }

    void SetupSlider(const size_t slider_idx, const double min, const double max, const double value,
                     const size_t num_dec = 3, const QString label_text = QString()){
      STD_INVALID_ARG_EM(!ui_is_init, "you must call SetupSlider() before InitUI() is called")
      STD_INVALID_ARG_EM(slider_vec.size() != 0, "you must call SetNumSlider() before SetupSlider() is called")
      // (min, max, value, num_decimal, abs_min, abs_max, step_size, label_text)
      slider_vec[slider_idx]->Init(min, max, value, num_dec, min, max, 1, label_text);
      //slider_vec[slider_idx]->SetMinimum(min);
      //slider_vec[slider_idx]->SetMaximum(max);
      //slider_vec[slider_idx]->SetValue(value);
    }

    double SliderValue(const size_t idx){
      return slider_vec[idx]->Value();
    }

    virtual void PlotCurves(){
      STD_INVALID_ARG_EM(false, "PlotCurves() function was not redefined in derived class")
    }

  public slots:
    void Replot(){
      STD_INVALID_ARG_E(ui_is_init)
      PlotCurves(); //must be defined by user (function that fills curve data)
      graph->qwt_plot->replot();
    }
};
#endif //HAVE_ADV_SLIDER_WIDGET

#endif //__QWT_GRAPH_H__

