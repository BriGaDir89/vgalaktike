#ifndef AMF_H
#define AMF_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <math.h>
#include <limits>

class AMF : public QObject {
    Q_OBJECT
    public:
        explicit AMF(QObject *parent = 0);
        int Decode( QByteArray byteArray_ );
    signals:

    public slots:

    private:
        quint8    reedByte( unsigned long pos_ );
        QVariant  readValue( void );
        QVariant  readStrictArray( void );
        QVariant  readDouble( void );
        QString   readAmf0String( void );
        QVariant  readAmf3Array( void );
        QVariant  readAmf3Object( void );
        QString   readAmf3String( void );
        long long readUInt29( void );
        bool      readBoolean( void );
        long long readUInt( int byteCount );
        QString   readUtf8( int byteLength );
    private:
        unsigned long pos;
        QByteArray byteArray;

        int version;
        QVector< QMap<QString, QVariant> > headers;
        QVector< QMap<QString, QVariant> > messages;

        QVector< QString  > buf_string;
        QVector< QVariant > buf_object;
};

#endif // AMF_H
