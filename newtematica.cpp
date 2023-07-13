#include "newtematica.h"
#include "ui_newtematica.h"

NewTematica::NewTematica(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewTematica)
{
    ui->setupUi(this);
    setWindowTitle("Formulario de Preguntas");

    // Configurar la tabla
    ui->tblLista->setColumnCount(2);
    QStringList titulo;
    titulo << "Pregunta" << "Respuesta";
    ui->tblLista->setHorizontalHeaderLabels(titulo);

    // Ajustar el ancho de la columna de preguntas
    ui->tblLista->setColumnWidth(PREGUNTAS, 450); // Ajusta el ancho en píxeles

    // Leer desde el archivo
    cargarPreguntas();
}

NewTematica::~NewTematica()
{
    delete ui;
}


void NewTematica::cargarPreguntas()
{
    // Verificar si el archivo existe
    QFile archivo(ARCHIVO);
    if (!archivo.exists())
        return;

    // cargar datos
    if (archivo.open(QFile::ReadOnly)) {
        QTextStream entrada(&archivo);
        int fila;
        while(!entrada.atEnd()){
            QString linea = entrada.readLine();
            QStringList datos = linea.split(";");
            //Agregar a la tabla
            fila = ui->tblLista->rowCount();
            ui->tblLista->insertRow(fila);
            ui->tblLista->setItem(fila, PREGUNTAS, new QTableWidgetItem(datos[PREGUNTAS]));
            ui->tblLista->setItem(fila, RESPUESTAS, new QTableWidgetItem(datos[RESPUESTAS]));
        }
        archivo.close();
    }
}

void NewTematica::cargarPreguntas(const QString& filePath)
{
    // Verificar si el archivo existe
    QFile archivo(filePath);
    if (!archivo.exists()) {
        QMessageBox::critical(this, "Cargar preguntas", "El archivo no existe.");
        return;
    }

    // Limpiar la tabla existente
    ui->tblLista->clearContents();
    ui->tblLista->setRowCount(0);

    // Cargar las preguntas del nuevo archivo
    if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream entrada(&archivo);
        int fila = 0;
        while (!entrada.atEnd()) {
            QString linea = entrada.readLine();
            QStringList datos = linea.split(";");
            if (datos.size() == 2) {
                ui->tblLista->insertRow(fila);
                ui->tblLista->setItem(fila, PREGUNTAS, new QTableWidgetItem(datos[0]));
                ui->tblLista->setItem(fila, RESPUESTAS, new QTableWidgetItem(datos[1]));
                fila++;
            }
        }
        archivo.close();
    } else {
        QMessageBox::critical(this, "Cargar preguntas", "No se pudo abrir el archivo.");
    }
}



void NewTematica::on_btn_Ingresar_clicked()
{
    // Crear y mostrar el dialogo
    Agregar pd(this);
    pd.setWindowTitle("Agregar Preguntas");
    // Abrir la ventana y evaluar respuesta
    int res = pd.exec();
    if (res == QDialog::Rejected){
        return;
    }
    // Recuperar el objeto del cuadro de dialogo
    Preguntas *p = pd.preguntas();
    //Agregar a la tabla
    int fila = ui->tblLista->rowCount();
    ui->tblLista->insertRow(fila);

    ui->tblLista->setItem(fila, PREGUNTAS, new QTableWidgetItem(p->pregunta()));
    ui->tblLista->setItem(fila, RESPUESTAS, new QTableWidgetItem(p->respuesta()));
}


void NewTematica::on_btn_CargaPreguntas_clicked()
{
    // Mostrar el diálogo para seleccionar un archivo de texto o CSV
    QString filePath = QFileDialog::getOpenFileName(this, "Seleccionar archivo", QString(), "Archivos de texto (*.txt *.csv )");

    // Verificar si se seleccionó un archivo
    if (!filePath.isEmpty()) {
        cargarPreguntas(filePath);
        QMessageBox::information(this, "Cargar archivo", "Archivo cargado exitosamente.");
    }
}


void NewTematica::on_btn_Guardar_clicked()
{
    // Verificar que existan datos para guardar
    int filas = ui->tblLista->rowCount();
    if (filas == 0) {
        QMessageBox::warning(this, "Guardar archivo", "No hay datos para guardar");
        return;
    }

    // Obtener el nombre del archivo ingresado por el usuario
    QString fileName = ui->ltxNombreTematica->text();

    // Verificar que se haya ingresado un nombre de archivo válido
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Guardar archivo", "Debe ingresar un nombre de archivo válido");
        return;
    }

    // Agregar la extensión .bin al nombre del archivo
    QString binFilePath = fileName + ".bin";

    // Obtener la ruta de la carpeta de la aplicación
    QString folderPath = QCoreApplication::applicationDirPath();

    // Combinar la ruta de la carpeta y el nombre del archivo
    QString selectedFilePath = folderPath + "/" + binFilePath;

    // Abrir el archivo binario en modo escritura
    QFile archivo(selectedFilePath);
    if (archivo.open(QIODevice::WriteOnly)) {
        QDataStream salida(&archivo);

        for (int i = 0; i < filas; i++) {
            QTableWidgetItem *pregunta = ui->tblLista->item(i, PREGUNTAS);
            QTableWidgetItem *respuesta = ui->tblLista->item(i, RESPUESTAS);

            QString preguntaText = pregunta->text();
            QString respuestaText = respuesta->text();

            // Escribir los datos en el archivo binario
            salida << preguntaText << respuestaText;
        }

        archivo.close();
        QMessageBox::information(this, "Guardar archivo", "Preguntas guardadas exitosamente en el archivo binario.");
    } else {
        QMessageBox::critical(this, "Guardar archivo", "No se puede escribir en el archivo binario.");
        return;
    }


    close();
}



void NewTematica::on_btn_Eliminar_clicked()
{
    int filaSeleccionada = ui->tblLista->currentRow();

    if (filaSeleccionada < 0) {
        return;
    }

    ui->tblLista->removeRow(filaSeleccionada);
}


void NewTematica::on_btn_Editar_clicked()
{
    int filaSeleccionada = ui->tblLista->currentRow();

    if (filaSeleccionada < 0) {
        return;
    }

    // Obtener los datos de la fila seleccionada
    QTableWidgetItem *preguntaItem = ui->tblLista->item(filaSeleccionada, PREGUNTAS);
    QTableWidgetItem *respuestaItem = ui->tblLista->item(filaSeleccionada, RESPUESTAS);

    // Verificar si se seleccionó una fila válida
    if (preguntaItem && respuestaItem) {
        // Obtener los textos actuales de la pregunta y respuesta
        QString preguntaActual = preguntaItem->text();
        QString respuestaActual = respuestaItem->text();

        // Crear y mostrar el cuadro de diálogo para editar la pregunta y respuesta
        Agregar pd(this);
        pd.setWindowTitle("Editar Pregunta");
        pd.setPregunta(preguntaActual);
        pd.setRespuesta(respuestaActual);

        int res = pd.exec();
        if (res == QDialog::Rejected) {
            return;
        }

        // Obtener los nuevos valores de pregunta y respuesta desde el cuadro de diálogo
        QString nuevaPregunta = pd.getPregunta();
        QString nuevaRespuesta = pd.getRespuesta();

        // Actualizar los datos en la tabla
        preguntaItem->setText(nuevaPregunta);
        respuestaItem->setText(nuevaRespuesta);

        QMessageBox::information(this, "Editar Pregunta", "Pregunta editada exitosamente.");
    }
}


void NewTematica::on_btn_Cerrar_clicked()
{
    close();
}

