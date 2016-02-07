#include "amf.h"

AMF::AMF(QObject *parent) : QObject(parent)
{
    pos = 0;
}
//--------------------------------------------------------------------------------------
int AMF::Decode( QByteArray byteArray_ )
{
    /*QString str = "";
    for( int i = 0; i < byteArray_.count(); i ++ )
    {
        if( QString::number(quint8(byteArray_.at(i)), 16).length() == 1 )
            str += "0" + QString::number(quint8(byteArray_.at(i)), 16) + "_";
        else
            str += QString::number(quint8(byteArray_.at(i)), 16) + "_";
    }
    qDebug() << "byteArray: " << str;
    */
    byteArray = byteArray_;
    //qDebug() << "ByteArray: " << byteArray;
    version = 0; readUInt( 2 );
    //qDebug() << "version: " << version;

    for( int headerCount = readUInt(2); headerCount --; )
    {
        QMap<QString, QVariant> tmp_header;
        tmp_header[ "name"           ] = readAmf0String();
        tmp_header[ "mustUnderstand" ] = readBoolean();
        tmp_header[ "byteLength"     ] = readUInt(4);
        ///tmp_header[ "value"          ] = readValue();
        headers.append( tmp_header );
    }
    //qDebug() << "headers: " << headers;

    int messageCount = readUInt( 2 );
    //qDebug() << "messageCount: " << messageCount;
    for( ; messageCount--; )
    {
        QMap<QString, QVariant> tmp_message;
        tmp_message[ "targetURI"   ] = readAmf0String();
        //qDebug() << messageCount << " => targetURI:   " << tmp_message[ "targetURI" ];
        tmp_message[ "responseURI" ] = readAmf0String();
        //qDebug() << messageCount << " => responseURI: " << tmp_message[ "responseURI" ];
        tmp_message[ "byteLength"  ] = readUInt(4);
        //qDebug() << messageCount << " => byteLength:  " << tmp_message[ "byteLength" ];
        tmp_message[ "body"        ] = readValue();
        messages.append( tmp_message );
    }

    qDebug()<< "AMF MESSAGES: " << messages;

    return version;
}
//--------------------------------------------------------------------------------------
quint8 AMF::reedByte( unsigned long pos_ )
{
    //qDebug() << "[ "<< pos_ << " ] : " << QString::number(quint8(byteArray.at(pos_)), 16) << quint8(byteArray.at(pos_));
    return quint8(byteArray.at( pos_ ));
}
//--------------------------------------------------------------------------------------
QVariant AMF::readValue( void )
{
    int marker = int( reedByte( pos++ ) );
    if( marker == 17 )
    {
        version = 3;
        marker = int( reedByte( pos++ ) );
    }
    QVariant result;
    if( version == 0 )
    {
        switch( marker )
        {
            case 10: {
                //qDebug() << "=> readValue: AMF0: readStrictArray, maeker: 10";
                result = readStrictArray();
            } break;
            default: { qDebug() << "readValue: AMF0: default: marker: " << marker; }
        }
    }
    else
    {
        switch( marker )
        {
            //case  0: { result = 0; } break;
            case  1: { /*qDebug() << "=> readNull,       maeker: 1";*/  result = QVariant( 0 );   } break;
            //case  2: { result = 0; } break;
            //case  3: { result = 0; } break;
            case  4: { /*qDebug() << "=> readUInt29,     maeker: 4";*/  result = readUInt29();     } break;
            case  5: { /*qDebug() << "=> readDouble,     maeker: 5";*/  result = readDouble();     } break;
            case  6: { /*qDebug() << "=> readAmf3String, maeker: 6";*/  result = readAmf3String(); } break;
            //case  7: { result = 0; } break;
            //case  8: { result = 0; } break;
            case  9: { /*qDebug() << "=> readAmf3Array,  maeker: 9";*/  result = readAmf3Array();  } break;
            case 10: { /*qDebug() << "=> readAmf3Object, maeker: 10";*/ result = readAmf3Object(); } break;
            //case 11: { result = 0; } break;
            //case 12: { result = 0; } break;
            default: { qDebug() << "readValue: AMF3: default: marker: " << marker; }
        }
    }

    return result;
}
//--------------------------------------------------------------------------------------
QVariant AMF::readStrictArray( void )
{
     int len = readUInt(4);
     QVariantList arr;

     while( len-- ) {
         arr.append( readValue() );
     }

     buf_object.append( arr );

     return arr;
}
//--------------------------------------------------------------------------------------
QVariant AMF::readDouble( void )
{
    int byte1       = reedByte( pos++ );
    int byte2       = reedByte( pos++ );
    double sign        = (byte1 >> 7) ? -1 : 1;
    double exponent    = ( ( ( byte1 & 0x7F ) << 4 ) | ( byte2 >> 4 ) );
    double significand = (byte2 & 0x0F);
    double hiddenBit   = exponent ? 1 : 0;
    double i           = 6;
    double twoPow8     = pow(2, 8);
    double twoPowN52   = pow(2, -52);
    while( i-- ) {
        significand = (significand * twoPow8) + reedByte( pos++ );
    }
    if( !exponent )
    {
        if( !significand ) {
            return 0;
        }
        exponent = 1;
    }
    if( exponent == 0x7FF ) {
        return significand ? qlonglong(NULL) : (std::numeric_limits<double>::infinity() * sign);
    }
    return qlonglong( sign * pow(2, exponent - 0x3FF)
                           * (hiddenBit + twoPowN52 * significand) );
}
//--------------------------------------------------------------------------------------
QVariant AMF::readAmf3Array( void )
{
    long long header = readUInt29();
    int count = 0;
    if( header & 1 )
    {
        count = (header >> 1);
        QString key = readAmf3String();
        if( !key.isEmpty() ) // если асоциативный массив
        {
            QMap<QString, QVariant> tmp_array;
            do {
                tmp_array[ key ] = readValue();
                //qDebug() << "readAmf3Array: tmp_array[ " << key << "]: " << tmp_array[ key ];
            } while( !( key = readAmf3String() ).isEmpty() );
            return tmp_array;
        }
        else {
            QVariantList tmp_list;
            for( int i = 0; i < count; i++ )
            {
                tmp_list.append( readValue() );
               // qDebug() << "readAmf3Array: tmp_list[ " << i << "]: " << tmp_list.at( i );
            }
            return tmp_list;
        }
    }
    return QVariant();
}
//--------------------------------------------------------------------------------------
QVariant AMF::readAmf3Object( void )
{
    long long header = readUInt29();
    //qDebug() << "readAmf3Object: header: " << header;
    QString className;
    bool dynamic = false;
    QStringList members;
    int memberCount = 0;
    QMap<QString, QVariant> tmp_object;
    if( header & 1 )
    {
        long long headerLast3Bits = header & 0x07;
        if( headerLast3Bits == 3 )
        {
            className = readAmf3String();
            //qDebug() << "readAmf3Object: className: " << className;
            dynamic = bool( header & 0x08 );
            //qDebug() << "readAmf3Object: dynamic: " << dynamic;

            memberCount = (header >> 4);
            for( int i = 0; i < memberCount; i ++ )
            {
                members.append( readAmf3String() );
            }
            /*
            tmp_object[ "className" ] = className;
            tmp_object[ "dynamic"   ] = dynamic;
            tmp_object[ "members"   ] = 0;*/
        }
        else
        {
            if( (header & 0x03) == 1 )
            {
                qDebug() << "readAmf3Object: (header & 0x03) == 1";
            }
            else
            {
                if( headerLast3Bits == 7 )
                {
                    qDebug() << "readAmf3Object: headerLast3Bits == 7";
                }
            }
        }

        for( int i = 0; i < memberCount; i ++ )
        {
            tmp_object[ members.at( i ) ] = readValue();
        }

        if( dynamic )
        {
            QString key;
            while( !( key = readAmf3String() ).isEmpty() )
            {
                tmp_object[ key ] = readValue();
                //qDebug() << "[ " << key << " ] = " << tmp_object[ key ];
            }
            //qDebug() << "readAmf3Object: tmp_object: dynamic: true :" << tmp_object;
            buf_object.append( QVariant( tmp_object ) );
            return QVariant( tmp_object );
        }
    }
    else
    {
        //qDebug() << "readAmf3Object: return buf_object.at( header >> 1 ): " << (header >> 1);
        buf_object.append( QVariant( tmp_object ) );
        return buf_object.at( header >> 1 );
    }

    buf_object.append( QVariant( tmp_object ) );
    return tmp_object;
}
//--------------------------------------------------------------------------------------
QString AMF::readAmf0String( void )
{
    return readUtf8( readUInt( 2 ) );
}
//--------------------------------------------------------------------------------------
QString AMF::readAmf3String( void )
{
    long header = readUInt29();
    if( header & 1 )
    {
        QString result = readUtf8(header >> 1);
        if( !result.isEmpty() ) buf_string.append( result );
        return result;
    }
    else
    {
        return buf_string.at( header >> 1 );
    }
    //return QString();
}
//--------------------------------------------------------------------------------------
long long AMF::readUInt29( void )
{
    long long value = long( reedByte( pos++ ) );
    long long nextByte;
    if (value & 0x80)
    {
        nextByte = long( reedByte( pos++ ) );
        value = ((value & 0x7F) << 7) | (nextByte & 0x7F);
        if (nextByte & 0x80)
        {
            nextByte = long( reedByte( pos++ ) );
            value = (value << 7) | (nextByte & 0x7F);
            if (nextByte & 0x80)
            {
                nextByte = long( reedByte( pos++ ) );
                value = (value << 8) | nextByte;
            }
        }
    }
    //qDebug() << "readUInt29: value: " << value;
    return value;
}
//--------------------------------------------------------------------------------------
bool AMF::readBoolean( void )
{
   return bool( reedByte( pos++ ) );
}
//--------------------------------------------------------------------------------------
long long AMF::readUInt( int byteCount )
{
    long long result = reedByte( pos++ );
    for( int i = 1; i < byteCount; i ++ )
        result = ( result << 8) | reedByte( pos++ );
    //qDebug() << "readUInt: " << result;
    return result;
}
//--------------------------------------------------------------------------------------
QString AMF::readUtf8( int byteLength )
{
    unsigned long end = pos + byteLength;
    //qDebug() << "readUtf8: start: " << pos << " end: " << end;
    int byteCount;
    QString result = "";
    while( pos < end  ) {
        int charCode = int( reedByte( pos++ ) );
        if( charCode > 127 ) {
            if( charCode > 239 ) {
                byteCount = 4;
                charCode = (charCode & 0x07 );
            }
            else
                if( charCode > 223 ) {
                    byteCount = 3;
                    charCode = (charCode & 0x0F);
                }
                else {
                    byteCount = 2;
                    charCode = (charCode & 0x1F);
                }
            while( --byteCount )
                charCode = ((charCode << 6) | (reedByte( pos++ ) & 0x3F));
        }
        result += QString( charCode );
    }
    //qDebug() << "readUtf8: result: " << result;
    return result;
}
