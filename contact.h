#ifndef CONTACT_H
#define CONTACT_H

#include <QString>
#define CONTACT_SEPARATOR ';'


class Contact{
private:
    QString m_firstname;
    QString m_lastname;
    QString m_email;
    QString m_numberphone;
    QString m_commentary;
    QString m_id;
    static QStringList m_ids;

    Contact( QString firstname, QString lastname, QString email, QString phone, QString commentary);
    Contact(QString infos);
    static QString createID(QString email, QString numberphone);

public:
    static Contact* createContact(QString infos = "");
    static Contact* createContact(QString firstname, QString lastname, QString email, QString phone, QString commentary);


    QString firstname() const;
    void setFirstname(const QString &firstname);
    QString lastname() const;
    void setLastname(const QString &lastname);
    QString email() const;
    void setEmail(const QString &email);
    QString numberphone() const;
    void setNumberphone(const QString &numberphone);
    QString commentary(bool encode = false) const;
    void setCommentary(const QString &commentary);
    void updateID();

    QString getLine();
};

#endif // CONTACT_H
