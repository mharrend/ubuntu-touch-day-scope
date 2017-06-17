/*
 * Copyright (C) 2015 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kyle Nitzsche <kyle.nitzsche@canonical.com>
 *
 */

#include "query.h"
#include "i18n.h"
#include "config.h"

#include <locale.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/VariantBuilder.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/ScopeExceptions.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QIODevice>
#include <QMessageAuthenticationCode>

#include <libintl.h>
#include <list>
#include <boost/algorithm/string/trim.hpp>

#include <unistd.h>

namespace us = unity::scopes;
using namespace std;

const static string DAY = R"(
{
    "schema-version": 1,
    "template": {
        "category-layout": "grid",
        "card-size": "small",
        "card-layout": "horizontal",
        "non-interactive": "true"
    },
    "components": {
        "title": "title",
        "art": "art",
        "attributes": {
            "field": "attributes",
            "max-count": 8
        }
    }
}
)";

Query::Query(const us::CannedQuery &query, const us::SearchMetadata & metadata_, Scope & scope, QString const& cacheDir_) :
    SearchQueryBase(query, metadata_),
    cannedQuery(query),
    scope_(scope),
    cacheDir_(cacheDir_)
{
    lunarData_filename = QString("%1/LunarData.json").arg(cacheDir_);
    lunarData_f.setFileName(lunarData_filename);
    sunData_filename = QString("%1/SunData.json").arg(cacheDir_);
    sunData_f.setFileName(sunData_filename);
    lunarLastRefresh_filename = QString("%1/LastRefresh.txt").arg(cacheDir_);
    lunarLastRefresh_f.setFileName(lunarLastRefresh_filename);
    sunsLastRefresh_filename = QString("%1/SunLastRefresh.txt").arg(cacheDir_);
    sunsLastRefresh_f.setFileName(sunsLastRefresh_filename);
}

Query::~Query()
{
}

void Query::cancelled()
{
}

QString strong(QString str){
    return QString("<strong>%1</strong>").arg(str);
}

bool Query::useNetwork(QFile &data, QFile &lastRefresh, data_t t)
{

    if (!search_metadata().has_location() || !search_metadata().internet_connectivity() == us::QueryMetadata::Connected)
	return false;

    if (t == data_t::suns)
	qDebug() << "==== useNetwork() for suns";
    else
	qDebug() << "==== useNetwork() for lunar";

    data.close();
    if (!data.open(QIODevice::ReadOnly))
    {
        qWarning() << "===== The JSON file missing: " << data.fileName();
        data.close();
        data.remove();
        return true;
    }
    lastRefresh.close();
    if (!lastRefresh.open(QIODevice::ReadOnly))
    {
        qWarning() << "===== The refresh cache file missing: " << lastRefresh.fileName();
        lastRefresh.remove();
        return true;
    }
    else
    {
        QDate today = QDate::currentDate();
        QByteArray cachedDate_ba = lastRefresh.readAll();
        lastRefresh.close();
        QString cachedDate_qstr = QString(cachedDate_ba);
        QDate cachedDate = QDate::fromString(cachedDate_qstr.trimmed(), "yyyy-MM-dd");
        if (!cachedDate.isValid())
        {
            qWarning() << "===== Timestamp in refresh file is invalid: " << lastRefresh.fileName();
            lastRefresh.remove();
            return true;
        }
        if (t == data_t::suns)
        {
            if (cachedDate != today)
            {
                qDebug() << "==== Suns cache date not today, so use network.";
                lastRefresh.remove();
                data.remove();
                return true;
            }
        }
        else if (t == data_t::lunar )
        {
            QDate oneMonthAgo = today.addDays(-31);
            if ((cachedDate == oneMonthAgo) || (cachedDate < oneMonthAgo))
            {
                qDebug() << "==== Lunar timestamp indicates we need to refresh cache from web.";
                lastRefresh.remove();
                data.remove();
                return true;
            }
        }
    }
    return false;
}

void Query::getFromNetwork(QFile &data, QFile &lastRefresh, data_t t)
{
    if (t == data_t::suns)
	qDebug() << "==== useNetwork() for suns";
    else
	qDebug() << "==== useNetwork() for lunar";

    if (search_metadata().has_location() && search_metadata().internet_connectivity() == us::QueryMetadata::Connected)
    {
        /* Old Moon phase API
        if (!haveSignature)
        {
            makeSignature();
        }
        */
        if (t == data_t::suns)
        {
            suns = getSuns(data, lastRefresh);
        }
        else if (t == data_t::lunar)
        {
            phase = getLunarPhase(data, lastRefresh);
        }
    }
    else
    {
        qDebug() << "==== DAY. no location";
        suns["sunrise"] = QString::fromStdString(_("No Information"));
        suns["sunset"] = QString::fromStdString(_("No Information"));
    }
}

void Query::getFromLocal(QFile &data, QFile &lastRefresh, data_t t)
{
    if (t == data_t::suns)
	qDebug() << "==== getFromLocal() for suns";
    else
	qDebug() << "==== getFromLocal() for lunar";

    data.close();
    if (data.open(QIODevice::ReadOnly))
    {
        QByteArray ba = data.readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(ba, &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "Cannot json parse from cache file: " << data.fileName();
            data.remove();
        }
        else
        {
            if (t == data_t::suns) 
            {
                QJsonObject obj = doc.object();
                if (obj.contains("sunrise") && obj.contains("sunset"))
                {
                    QString sunr = obj["sunrise"].toString();
                    suns["sunrise"] = sunr.split(" ")[1];
                    QString sunset = obj["sunset"].toString();
                    suns["sunset"] = sunset.split(" ")[1];
                }
                else
                {
                    getFromNetwork(data, lastRefresh, data_t::suns);
                    lastRefresh.remove();
                    return;
                }
            }
            else if (t == data_t::lunar)
            {
                QJsonObject doc_object = doc.object();
                if (doc_object.contains("locations"))
                {
                    phase = parseLunarPhase(doc);
                    if (phase.isEmpty())
                    {
                        qWarning() << "===== Lunar cache file invalid. Request moon phase from network.";
                        getFromNetwork(data, lastRefresh, data_t::lunar);
                    }
                }
                else
                {
                    getFromNetwork(data, lastRefresh, data_t::lunar);
                    lastRefresh.remove();
                    return;
                }
        
            }
        }
    }
    else
    {
        if (t == data_t::suns) 
        {
            getFromNetwork(data, lastRefresh, data_t::lunar);
        }
        else if (t == data_t::lunar)
        {
            getFromNetwork(data, lastRefresh, data_t::lunar);
        }
    }
}

void Query::run(us::SearchReplyProxy const& reply)
{

    mUserAgent = QString("%1 (Ubuntu)").arg(SCOPE_PACKAGE);
    QString::fromStdString(setlocale(LC_ALL, ""));
    service = "astronomy";
    try
    {
        us::Location loc = search_metadata().location();
        lat = QString::number(loc.latitude());
        lng = QString::number(loc.longitude());

        QString locale = QString::fromStdString(search_metadata().locale());
        QStringList parts = locale.split("_");
        lang = parts[0];
    }
    catch (unity::Exception const &e)
    {
        qWarning() << "Location object not found and exception is caught: " << QString::fromStdString(e.what());
    }

    if (useNetwork(sunData_f, sunsLastRefresh_f, data_t::suns))
    {
	getFromNetwork(sunData_f, sunsLastRefresh_f, data_t::suns);
    } else {
	getFromLocal(sunData_f, sunsLastRefresh_f, data_t::suns);
   }

    if (useNetwork(lunarData_f, lunarLastRefresh_f, data_t::lunar))
    {
        getFromNetwork(lunarData_f, lunarLastRefresh_f, data_t::lunar);
    }
    else
    {
        getFromLocal(lunarData_f, lunarLastRefresh_f, data_t::lunar);
    }

    if (QString::fromStdString(phase.toStdString()).isEmpty())
    {
        phase = "No Information";
    }
    else
    {
        phase = _(phase.toStdString().c_str());
        //correct english. Note that current translations provide corrections 
        if (phase == "waningcrescent") 
            phase = "Waning crescent";
        else if (phase == "thirdquarter") 
            phase = "Third quarter";
        else if (phase == "waninggibbous")
            phase = "Waning gibbous";
        else if (phase == "fullmoon" )
            phase = "Full moon";
        else if (phase == "waxinggibbous")
            phase = "Waxing gibbous";
        else if (phase == "firstquarter")
            phase = "First quarter";
        else if (phase == "waxingcrescent")
            phase = "Waxing crescent";
        else if (phase == "newmoon")
            phase = "New moon";
    }

    // Initial date info
    auto date_info_cat = reply->register_category("date", "", "", us::CategoryRenderer(DAY));

    us::CategorisedResult date_res(date_info_cat);
    date_res.set_uri("http://ubuntu.com");

    QDate today = QDate::currentDate();

    QString mo = today.toString("MMMM");
    mo = mo.replace(0,1,mo[0].toUpper());
    QString day = today.toString("dddd");
    day = day.replace(0,1,day[0].toUpper());
    QString title = QString("%1, %2").arg(mo, day);
    date_res.set_title(strong(title).toStdString());
    QString week_no = QString("%1 %2").arg(QString::fromStdString(_("Week"))).arg(today.weekNumber());

    string icon_path =
        QString("%1/images/calendar-app_day-%2.png").arg(QString::fromStdString(scope_.scope_directory())).arg(today.toString("dd")).toStdString();
    date_res.set_art(icon_path);

    us::VariantBuilder builder;
    builder.add_tuple({
        {"value", us::Variant(strong(week_no).toStdString())}
    });
    builder.add_tuple({
        {"value",us:: Variant("")}
    });

    QString sunrise = QString::fromStdString(_("Sunrise"));
    QString sunset = QString::fromStdString(_("Sunset"));
    QString sunrise_time = QString::fromStdString(_("No Information"));
    QString sunset_time = QString::fromStdString(_("No Information"));

    if (search_metadata().has_location() && search_metadata().internet_connectivity() == us::QueryMetadata::Connected)
    {
        qDebug() << "==== we have location && network";
        us::Location loc = search_metadata().location();
        if (suns.size() >= 2) {
            sunrise_time = suns["sunrise"];
            sunset_time = suns["sunset"];
        }
    }
    else
    {
        qDebug() << "==== DAY. no location";
        sunrise_time = _("No Information");
        sunset_time = _("No Information");
    }

    builder.add_tuple({
        {"value", us::Variant(strong(QString("%1: %2").arg(sunrise).arg(sunrise_time)).toStdString())}
    });
    builder.add_tuple({
        {"value", us::Variant("")}
    });
    builder.add_tuple({
        {"value", us::Variant(strong(QString("%1: %2").arg(sunset).arg(sunset_time)).toStdString())}
    });
    builder.add_tuple({
        {"value", us::Variant("")}
    });
    if (search_metadata().has_location() && search_metadata().internet_connectivity() == us::QueryMetadata::Connected)
    {
        //TRANSLATORS: if there is one word for "phase of moon" please use it here
        QString phase_label = strong(QString::fromStdString(_("Moon")));
        builder.add_tuple({
            {"value", us::Variant(strong(QString("%1: %2").arg(phase_label).arg(phase)).toStdString())}
        });
        builder.add_tuple({
            {"value", us::Variant("")}
        });
    }
    date_res["attributes"] = builder.end();
    reply->push(date_res);

}

QMap<QString,QString> Query::getSuns(QFile &cache_f, QFile &lastRefresh) {
    qDebug() << "=== DAY getSuns()";
    qDebug() << "=== DAY lat long before suns: " << lat << " " << lng;

    cache_f.remove();
    lastRefresh.remove();

    QString lat_ = lat;
    QString lng_ = lng;
    if (lat_.startsWith("-"))
        lat_.replace(0,1,"%2D");
    else
        lat_ = "%2B" + lat;
    if (lng_.startsWith("-"))
        lng_.replace(0,1,"%2D");
    else
        lng_ = "%2B" + lng;
    qDebug() <<"==== DAY lat long aftere suns: " << lat_ << " " << lng_;

    QMap<QString,QString> suns;

    QString uri_s = QString("http://api.geonames.org/timezoneJSON?lat=%1&lng=%2&username=unityapi").arg(lat, lng);
    QUrl url = QUrl(uri_s);
    qDebug() <<"==== DAY get sunn url:" << url.toString();
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);

    QNetworkAccessManager manager;
    QEventLoop loop;

    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&manager, &QNetworkAccessManager::finished,[&lastRefresh, &cache_f, &suns, this](QNetworkReply *msg) {

        QByteArray data = msg->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            qCritical() << "Failed to parse server data: " << err.errorString();

	if (cache_f.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
	{
            cache_f.write(doc.toJson());
            cache_f.close();
            QDate today = QDate::currentDate();
            QString today_qstr = today.toString("yyyy-MM-dd");
            if (lastRefresh.open(QIODevice::ReadWrite | QIODevice::Truncate))
            {
                lastRefresh.write(today_qstr.toUtf8());
                lastRefresh.close();
            }
	}
        QJsonObject obj = doc.object();
        QString sunr = obj["sunrise"].toString();
        suns["sunrise"] = sunr.split(" ")[1];
        QString sunset = obj["sunset"].toString();
        suns["sunset"] = sunset.split(" ")[1];

    });

    QNetworkDiskCache *cache = new QNetworkDiskCache();
    cache->setCacheDirectory(QString::fromStdString(scope_.cache_directory()));
    manager.setCache(cache);
    manager.get(request);
    loop.exec();
    delete cache;

    return suns;
}
void Query::makeSignature() {
    return;    
    /* Old Moon phase API
    haveSignature = true;
    QDateTime dtCurrent = QDateTime::currentDateTime();
    QDateTime dtUTC = dtCurrent.toUTC(); //UTC iso format ends in 'Z'. Use utc to avoid tiemzone confusion
    timestampUtc = dtUTC.toString(Qt::ISODate);
    timestampUtcEncoded = QUrl::toPercentEncoding(timestampUtc);

    QMessageAuthenticationCode code(QCryptographicHash::Sha1);
    code.setKey(secretKey);
    //qDebug() << "=== DAY accessKey: " << accessKey;
    //qDebug() << "=== DAY service: " << service;
    //qDebug() << "=== DAY timestapUtc: " << timestampUtc.toUtf8();
    //qDebug() << "=== DAY timestapUtcEncoded: " << timestampUtcEncoded;

    QByteArray msg = accessKey + service + timestampUtc.toUtf8();
    code.addData(msg);
    signature = QString(code.result().toBase64());
    qDebug() << "=== DAY signature: " << signature;
    signatureEncoded = QUrl::toPercentEncoding(signature);
    qDebug() << "=== DAY signatureEncoded: " << signatureEncoded;
    return;
    */
}

QString Query::formatToday()
{
    QDate today = QDate::currentDate();
    QString startYear = QString::number(today.year());
    QString startMonth = QString::number(today.month());
    if (startMonth.size() == 1)
    {
        startMonth = "0" + startMonth;
    }
    QString startDay = QString::number(today.day());
    if (startDay.size() == 1)
    {
        startDay = "0" + startDay;
    }
    QString startdt = QString("%1-%2-%3").arg(startYear, startMonth, startDay);

    return startdt;
}

QString Query::getLunarPhase(QFile &data, QFile &lastRefresh) {

    data.remove();
    lastRefresh.remove();

    return QString("Needs new API Provider.");
    /*
    try
    {
        us::Location loc = search_metadata().location();
    }
    catch (unity::Exception const &e)
    {
        qDebug() << "=== DAY LOC. no location";
        return QString::fromStdString(_("No Information"));
    }

    //timeanddate.com state:
    //Coordinates in decimal format. Directly concatenate the coordinates including the leading sign, first the latitude, then the longitude. Negative latitudes indicate southern hemisphere, whereas positive values for the latitude indicate northern hemisphere. For longitude, negative values indicate the western hemisphere and positive values the eastern hemisphere.
    //
    //Example: +58.96+5.67
    QString lat_ = lat;
    QString lng_ = lng;
    if (lat_.startsWith("-"))
        lat_.replace(0,1,"%2D");
    else
        lat_ = "%2B" + lat;
    if (lng_.startsWith("-"))
        lng_.replace(0,1,"%2D");
    else
        lng_ = "%2B" + lng;
    QString latLng = lat_ + lng_;

    QString startdt = formatToday();
    QDate lastDay = QDate::fromString(startdt.trimmed(), "yyyy-MM-dd").addDays(30);
    QString endYear = QString::number(lastDay.year());
    QString endMonth = QString::number(lastDay.month());
    if (endMonth.size() ==1)
    {
        endMonth = "0" + endMonth;
    }
    QString endDay = QString::number(lastDay.day());
    if (endDay.size() == 1)
    {
        endDay = "0" + endDay;
    }
    QString enddt = QString("%1-%2-%3").arg(endYear, endMonth, endDay);

    QString object = "moon";
    QString isotime = "0";
    QString types = "phase";
    QUrl url = QString("//api.xmltime.com/%1").arg(QString(service));
    url.setScheme("https");
    QUrlQuery qry;
    qry.addQueryItem("accesskey", accessKey);
    qry.addQueryItem("timestamp", timestampUtcEncoded);
    qry.addQueryItem("signature", signatureEncoded);
    qry.addQueryItem("lang", lang);
    qry.addQueryItem("placeid", latLng);
    qry.addQueryItem("startdt", startdt);
    qry.addQueryItem("enddt", enddt);
    qry.addQueryItem("object", object);
    qry.addQueryItem("isotime", isotime);
    qry.addQueryItem("types", types);
    url.setQuery(qry);

    qDebug() << "==== DAY: lunar url:" << url.toString();

    QString phase;
    QEventLoop loop;
    QNetworkAccessManager manager;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&url, &phase, &data, &lastRefresh, this](QNetworkReply *netReply){
        QByteArray data_ba = netReply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data_ba, &err);

        if (err.error != QJsonParseError::NoError)
        {
            qCritical() << "Failed to parse server data: " << err.errorString();
        }
        else
        {
            //the json doc from the network call is valid
            QJsonObject json = doc.object();
            if (json.contains("errors"))
            {
                qWarning() << QString("=== DAY Failed attempt URL: %1").arg(url.toString());
            }

            if (data.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
            {
                data.write(doc.toJson());
                data.close();
            }
            QDate today = QDate::currentDate();
            QString today_qstr = today.toString("yyyy-MM-dd");
            if (lastRefresh.open(QIODevice::ReadWrite | QIODevice::Truncate))
            {
                lastRefresh.write(today_qstr.toUtf8());
                lastRefresh.close();
            }

            phase = parseLunarPhase(doc);
            return;
        }
    });

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", mUserAgent.toStdString().c_str());
    request.setRawHeader("Content-Type", "application/rss+xml, text/xml");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    manager.get(request);
    loop.exec();
    qDebug() << "==== DAY: phase: " << phase;
    return phase;
    */
}

QString Query::parseLunarPhase(QJsonDocument &doc)
{
    qDebug() << "==== parseLunarPhase()";

    QString lunarPhase;
    QJsonObject obj = doc.object();
    QJsonArray locations_a = obj["locations"].toArray();   

    for(const auto &loc_ : locations_a)
    {
        QJsonObject loc_o = loc_.toObject();
        QJsonObject astronomy_o = loc_o["astronomy"].toObject();
        QJsonArray objects_a = astronomy_o["objects"].toArray();
        for(const auto &object_ : objects_a)
        {
            QJsonObject object_o = object_.toObject();
            if (object_o["name"].toString() == "moon")
            {
                QJsonArray days_a = object_o["days"].toArray();
                for(const auto &day_ : days_a)
                {
                    QJsonObject day_o = day_.toObject();
                    if (day_o["date"].toString() == formatToday())
                    {
                        lunarPhase = day_o["moonphase"].toString();
                        qDebug() << "==== date: " << day_o["date"].toString();
                        qDebug() << "==== moonphase: " << lunarPhase;
                    }
                }
            }
        }
    }

    return lunarPhase;
}
