#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QDebug>
#include <QFileDialog>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_contactsFileName = "contacts.txt";

    DatabaseConnect();
    DatabaseInit();

}

MainWindow::~MainWindow() {
    for (auto contact: m_contacts){
        delete contact;
    }
    delete ui;
}


void MainWindow::on_loadContactsButton_clicked()
{
    loadContacts();
}

void MainWindow::populateList()
{
    ui->listContacts->clear();
    for(const auto &contact: qAsConst(m_contacts)) {
        qDebug() << contact->email();
        ui->listContacts->addItem(contact->firstname() + " | " + contact->lastname());
    }
}

void MainWindow::updateList()
{
    ui->listContacts->clear();
    for(const auto &contact: qAsConst(m_contacts)) {
        if(contact->firstname()==getFind() || contact->lastname()==getFind() || contact->email()==getFind() ||
                contact->numberphone()==getFind() || contact->commentary()==getFind() ){
            ui->listContacts->addItem(contact->email());
        }
    }
}

QString MainWindow::getFind()
{
    return findSearch;
}

QString MainWindow::formatCommentary(const QString &commentary)
{
    QString tmp = commentary;
    tmp = tmp.replace("<eol>", "\n");
    return tmp;
}

void MainWindow::DatabaseConnect()
{
    const QString DRIVER("QSQLITE");

    if (QSqlDatabase::isDriverAvailable(DRIVER)) {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);

        db.setDatabaseName("zzContact.db");

        if (!db.open())
            qWarning() << "MainWindow::DatabaseConnect - ERROR: " << db.lastError().text();
    } else
        qWarning() << "MainWindow::DatabaseConnect - ERROR: no driver " << DRIVER << " available";
}

void MainWindow::DatabaseInit()
{
    QSqlQuery query("CREATE TABLE contact (id INTEGER PRIMARY KEY, lastname TEXT, firstname TEXT, email TEXT, numberphone TEXT, commentary TEXT)");

    if (!query.isActive()){
        qWarning() << "MainWindow::DatabaseInit - ERROR: " << query.lastError().text();
    }else{
        // fonction qui met 3 contacts si db existe pas
        DatabasePopulate();
        qWarning() << "db didn't exist, we have populate the db, enjoy";
    }
    populateListWithDB();
}

void MainWindow::DatabasePopulate()
{
    QSqlQuery query;

    if (!query.exec("INSERT INTO contact(lastname, firstname, email, numberphone, commentary) VALUES('Matas', 'Lucas', 'tqt@gmail.com', '0618863822', 'bh bg tu coco')"))
        qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();
    if (!query.exec("INSERT INTO contact(lastname, firstname, email, numberphone, commentary) VALUES('Lalande', 'Jason', 'bg@gmail.com', '0680786961', 'le plu bo')"))
        qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();
    if (!query.exec("INSERT INTO contact(lastname, firstname, email, numberphone, commentary) VALUES('Laurens', 'Leo', 'flemme@gmail.com', '0651846278', 'en vrai flemme')"))
        qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();

}

void MainWindow::populateListWithDB()
{

    QSqlQuery query;
    query.prepare( "SELECT * FROM contact" );

    if( !query.exec() )
    {
        qWarning() << "MainWindow::DatabasePopulateList - ERROR: " << query.lastError().text();
        return;
    }

    while( query.next() )
    {
         Contact* newContact = Contact::createContact(
                             query.value(1).toString(),
                             query.value(2).toString(),
                             query.value(3).toString(),
                             query.value(4).toString(),
                             query.value(5).toString());

         if(newContact != nullptr)
         {
            m_contacts.append(newContact);

         }
    }
    populateList();
}

bool MainWindow::DatabaseOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
     tr("Ouvrir une DB"), "C://Documents/");

    if(!fileName.isEmpty()){
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setHostName("localhost");
        db.setDatabaseName(fileName);
        if (!db.open()){
            qWarning() << "MainWindow::DatabaseOpen - ERROR: " << db.lastError().text();
            return false;
        }

        return true;
    }
    return false;
}

void MainWindow::on_listContacts_currentRowChanged(int currentRow)
{
    qDebug() << __FUNCTION__ << currentRow;
    if (currentRow == -1) return;
    m_currentContact = m_contacts.at(currentRow);
    populateForm();
}

void MainWindow::populateForm()
{
    ui->firstname_textEdit->setText(m_currentContact->firstname());
    ui->lastname_textEdit->setText(m_currentContact->lastname());
    ui->email_textEdit->setText(m_currentContact->email());
    ui->numberPhone_textEdit->setText(m_currentContact->numberphone());
    ui->commentary_textEdit->setText(m_currentContact->commentary());
}

void MainWindow::loadContacts()
{
    QFile contactFile(QFileDialog::getOpenFileName(this, "Ouvrir un fichier", "C://Documents/"));
    if (contactFile.open(QIODevice::ReadWrite | QIODevice::Text)){
        QTextStream flux(&contactFile);
        QString all = flux.readAll();
        QStringList lines(all.split("\n"));
        for (const auto &line : qAsConst(lines)) {
            if(line.isEmpty()) {
                continue;
            }
           Contact* newContact = Contact::createContact(line);
           if(newContact != nullptr)
           {
              m_contacts.append(newContact);
              m_currentContact = m_contacts.last();
              saveContacts();
           }
        }
        contactFile.close();
        populateList();
        ui->statusbar->showMessage(QString::number(m_contacts.length()) + " contacts chargés.");
    } else {
        QString error = "Impossible d'ouvrir le fichier " + m_contactsFileName;
        qCritical() << error;
        ui->statusbar->showMessage(error);
    }
}

void MainWindow::saveContacts()
{
    QSqlQuery query;

    query.prepare("INSERT INTO contact(lastname, firstname, email, numberphone, commentary) VALUES(:lastname, :firstname, :email, :numberphone, :commentary)");
    query.bindValue(":lastname", m_currentContact->lastname());
    query.bindValue(":firstname", m_currentContact->firstname());
    query.bindValue(":email", m_currentContact->email());
    query.bindValue(":numberphone", m_currentContact->numberphone());
    query.bindValue(":commentary", m_currentContact->commentary());

    if(query.exec()){
        qDebug() << "save Contact reussi";
    }else {
        qDebug() << "save Contact failed " << query.lastError().text();
    }
}

void MainWindow::saveFileContacts()
{
    QFile contactFile(QFileDialog::getOpenFileName(this, "Sauvegarder un fichier", "C://Documents/"));
    if (contactFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream flux(&contactFile);
        for (auto contact : m_contacts) {
            QString contactStr = contact->getLine();
            flux << contactStr;
        }
        contactFile.close();
        ui->statusbar->showMessage(QString::number(m_contacts.length()) + " contacts enregistrés.");
    } else {
        QString error = "Impossible de sauver le fichier " + m_contactsFileName;
        qCritical() << error;
        ui->statusbar->showMessage(error);
    }
}

void MainWindow::on_submit_clicked()
{
    qDebug() << __FUNCTION__;
    m_currentContact->setFirstname(ui->firstname_textEdit->text());
    m_currentContact->setLastname(ui->lastname_textEdit->text());
    m_currentContact->setEmail(ui->email_textEdit->text());
    m_currentContact->setNumberphone(ui->numberPhone_textEdit->text());
    m_currentContact->setCommentary(ui->commentary_textEdit->toPlainText());
    populateList();
    saveContacts();
}

void MainWindow::on_saveFileButton_clicked()
{
    saveFileContacts();
}

void MainWindow::on_addContact_clicked()
{
    m_contacts.append(Contact::createContact());
    populateList();
    ui->listContacts->setCurrentRow(m_contacts.length()-1);
}

void MainWindow::on_searchContact_clicked()
{
    QString text = ui->searchContact_text->text();

    if(text.isEmpty()) {
        ui->statusbar->showMessage("Veuillez rechercher un contact par son Nom, Prenom, Email, Numeros ou Commentaire");
        populateList();
        qCritical() << "Veuillez rechercher un contact par son Nom, Prenom, Email, Numeros ou Commentaire" ;
        return;
    } else {
        findSearch = text;
        ui->searchContact_text->clear();
        ui->statusbar->clearMessage();
        ui->statusbar->showMessage("Resultat pour la recherche de : " + findSearch );
        updateList();
    }
}

void MainWindow::on_btn_delete_clicked()
{
    deleteContact();
}

void MainWindow::deleteContact()
{
    QSqlQuery query;
    query.prepare("DELETE FROM contact WHERE lastname = (:lastname) AND firstname = (:firstname) AND email = (:email) AND numberphone = (:numberphone) AND commentary = (:commentary)");

    query.bindValue(":lastname", m_currentContact->lastname());
    query.bindValue(":firstname", m_currentContact->firstname());
    query.bindValue(":email", m_currentContact->email());
    query.bindValue(":numberphone", m_currentContact->numberphone());
    query.bindValue(":commentary", m_currentContact->commentary());

    if(query.exec()){
        qDebug() << "delete Contact reussi";
    }else {
        qDebug() << "delete Contact failed " << query.lastError().text();
    }

    populateList();
}

int MainWindow::findId()
{
    QSqlQuery query;

    query.prepare("SELECT * FROM contact WHERE lastname = (:lastname) AND firstname = (:firstname) AND email = (:email) AND numberphone = (:numberphone) AND commentary = (:commentary)");

    query.bindValue(":lastname", m_currentContact->lastname());
    query.bindValue(":firstname", m_currentContact->firstname());
    query.bindValue(":email", m_currentContact->email());
    query.bindValue(":numberphone", m_currentContact->numberphone());
    query.bindValue(":commentary", ui->commentary_textEdit->toPlainText());

    if(query.exec()){
        qDebug() << "select id Contact reussi";
        query.first();
    }else {
        qDebug() << "select id Contact failed " << query.lastError().text();
    }

    return query.value(0).toInt() ;
}

void MainWindow::updateContact(int id)
{
    QSqlQuery query;

    query.prepare("UPDATE contact SET lastname = (:lastname), firstname = (:firstname), email = (:email), numberphone = (:numberphone), commentary = (:commentary) WHERE id = (:id)");

    query.bindValue(":lastname", ui->lastname_textEdit->text());
    query.bindValue(":firstname", ui->firstname_textEdit->text());
    query.bindValue(":email", ui->email_textEdit->text());
    query.bindValue(":numberphone", ui->numberPhone_textEdit->text());
    query.bindValue(":commentary", ui->commentary_textEdit->toPlainText());
    query.bindValue(":id", id);

    if(query.exec()){
        qDebug() << "update id Contact reussi";
    }else {
        qDebug() << "update id Contact failed " << query.lastError().text();
    }
}

void MainWindow::on_update_clicked()
{
    updateContact(findId());
}

void MainWindow::on_Load_clicked()
{
    if(!DatabaseOpen()){
        return;
    }else{
        populateListWithDB();
    }
}
