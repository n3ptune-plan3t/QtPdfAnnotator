#include <QtWidgets>
#include <QtPdf>
#include <QtPdfWidgets>

// A custom QGraphicsScene to handle mouse events for drawing
class AnnotationScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit AnnotationScene(QObject *parent = nullptr)
        : QGraphicsScene(parent), m_drawing(false), m_penWidth(3) {
        m_penColor = Qt::red;
    }

    void setPenColor(const QColor &color) {
        m_penColor = color;
    }

    void setPenWidth(int width) {
        m_penWidth = width;
    }

protected:
    // Called when the mouse button is pressed
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_drawing = true;
            // Create a new path and move it to the start position
            m_currentPath = new QGraphicsPathItem();
            m_currentPath->setPen(QPen(m_penColor, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            QPainterPath path;
            path.moveTo(event->scenePos());
            m_currentPath->setPath(path);
            addItem(m_currentPath);
        }
        // Propagate the event to allow for selecting/moving items if needed
        QGraphicsScene::mousePressEvent(event);
    }

    // Called when the mouse is moved while a button is pressed
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        if (m_drawing && m_currentPath) {
            // Add a new line to the current path
            QPainterPath path = m_currentPath->path();
            path.lineTo(event->scenePos());
            m_currentPath->setPath(path);
        }
        QGraphicsScene::mouseMoveEvent(event);
    }

    // Called when the mouse button is released
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && m_drawing) {
            m_drawing = false;
            m_currentPath = nullptr;
        }
        QGraphicsScene::mouseReleaseEvent(event);
    }

private:
    bool m_drawing;
    QColor m_penColor;
    int m_penWidth;
    QGraphicsPathItem *m_currentPath;
};

// The main application window
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("PDF Annotator");
        setMinimumSize(800, 600);

        // --- Setup Graphics View and Scene ---
        // The QGraphicsView provides the "infinite canvas" feel
        m_view = new QGraphicsView(this);
        m_scene = new AnnotationScene(this);
        m_view->setScene(m_scene);
        
        // Allow panning by dragging with the middle mouse button
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
        // Better rendering quality
        m_view->setRenderHint(QPainter::Antialiasing);
        m_view->setRenderHint(QPainter::SmoothPixmapTransform);

        setCentralWidget(m_view);

        // --- Setup Toolbar ---
        QToolBar *toolbar = addToolBar("Main Toolbar");
        
        const QIcon openIcon = QIcon::fromTheme("document-open");
        QAction *openAction = toolbar->addAction(openIcon, "Open PDF");
        connect(openAction, &QAction::triggered, this, &MainWindow::openPdf);

        toolbar->addSeparator();

        const QIcon colorIcon = QIcon::fromTheme("preferences-color");
        QAction *colorAction = toolbar->addAction(colorIcon, "Pen Color");
        connect(colorAction, &QAction::triggered, this, &MainWindow::selectPenColor);
        
        // Add a spinner for pen width
        QSpinBox* penWidthSpinner = new QSpinBox();
        penWidthSpinner->setRange(1, 20);
        penWidthSpinner->setSuffix("px");
        penWidthSpinner->setValue(3);
        toolbar->addWidget(penWidthSpinner);
        connect(penWidthSpinner, &QSpinBox::valueChanged, m_scene, &AnnotationScene::setPenWidth);
        
        m_pdfDocument = nullptr;
    }

protected:
    // Handle zooming with the mouse wheel
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) {
            const qreal scaleFactor = 1.15;
            if (event->angleDelta().y() > 0) {
                // Zoom in
                m_view->scale(scaleFactor, scaleFactor);
            } else {
                // Zoom out
                m_view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            }
            event->accept();
        } else {
            QMainWindow::wheelEvent(event);
        }
    }


private slots:
    void openPdf() {
        QString filePath = QFileDialog::getOpenFileName(this, "Open PDF File", QString(), "PDF Files (*.pdf)");
        if (filePath.isEmpty()) {
            return;
        }

        // Clean up previous document and scene
        m_scene->clear();
        if (m_pdfDocument) {
            delete m_pdfDocument;
        }

        m_pdfDocument = new QPdfDocument(this);
        m_pdfDocument->load(filePath);

        if (m_pdfDocument->status() != QPdfDocument::Status::Ready) {
            QMessageBox::critical(this, "Error", "Failed to load PDF file.");
            return;
        }

        // Render each page and add it to the scene
        qreal yPos = 0.0;
        const qreal pageSpacing = 20.0;

        for (int i = 0; i < m_pdfDocument->pageCount(); ++i) {
            QSizeF pageSize = m_pdfDocument->pagePointSize(i);
            QImage image = m_pdfDocument->render(i, QSize(pageSize.width(), pageSize.height()));
            
            QGraphicsPixmapItem *pixmapItem = m_scene->addPixmap(QPixmap::fromImage(image));
            pixmapItem->setPos(0, yPos);

            // Add a light gray background to visualize page boundaries
            QGraphicsRectItem *background = new QGraphicsRectItem(0, 0, pageSize.width(), pageSize.height());
            background->setBrush(Qt::white);
            background->setPen(QPen(Qt::gray));
            background->setZValue(-1); // Ensure it's behind the PDF content
            background->setPos(0, yPos);
            m_scene->addItem(background);


            yPos += pageSize.height() + pageSpacing;
        }
    }
    
    void selectPenColor() {
        QColor color = QColorDialog::getColor(Qt::red, this, "Select Pen Color");
        if (color.isValid()) {
            m_scene->setPenColor(color);
        }
    }

    /*
    * --- HOW TO IMPLEMENT EXPORTING AND ADDING PAGES ---
    *
    * EXPORTING:
    * 1. Create a QPdfWriter instance: `QPdfWriter pdfWriter("output.pdf");`
    * 2. Create a QPainter to draw on the writer: `QPainter painter(&pdfWriter);`
    * 3. For each page in your application:
    * a. If it's an existing PDF page, you can render the original page onto the painter first.
    * This is the tricky part, as you'd ideally want vector data, but rendering the QImage
    * from the import step is a simpler start: `painter.drawImage(rect, pageImage);`
    * b. Then, iterate through all QGraphicsPathItems (your annotations) that are on that page.
    * c. Use `item->paint(&painter, option, nullptr);` to draw each annotation onto the PDF page.
    * d. After painting a page, call `pdfWriter.newPage();` to advance to the next page.
    *
    * ADDING PAGES:
    * 1. Create an action/button for "Add Page".
    * 2. When triggered, determine the position for the new page (e.g., after the current last page).
    * 3. Create a new blank page item. This could be a QGraphicsRectItem like the background
    * items we added in `openPdf`. `QGraphicsRectItem *newPage = new QGraphicsRectItem(0, 0, pageWidth, pageHeight);`
    * 4. Add this new rectangle to the scene at the correct y-position.
    * 5. You would need to shift all subsequent pages down to make room. This requires keeping
    * track of your page items in a list and updating their positions.
    */


private:
    AnnotationScene *m_scene;
    QGraphicsView *m_view;
    QPdfDocument *m_pdfDocument;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
