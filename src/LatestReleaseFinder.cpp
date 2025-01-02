/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * LatestReleaseFinder.h is part of Brewtarget, and is copyright the following authors 2024:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "LatestReleaseFinder.h"

#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include <QDebug>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVersionNumber>

#include "config.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_LatestReleaseFinder.cpp"

void LatestReleaseFinder::checkMainRespository() {

   //
   // Checking for the latest version involves requesting a JSON object from the GitHub API over HTTPS.
   //
   // Previously we used the Qt framework (QNetworkAccessManager / QNetworkRequest / QNetworkReply) to do the HTTP
   // request/response.  The problem with this is that, when something goes wrong it can be rather hard to diagnose.
   // Eg we had a bug that triggered a stack overflow in the Qt internals but there was only a limited amount of
   // logging we could add to try to determine what was going on.
   //
   // So now, instead, we use Boost.Beast (which sits on top of Boost.Asio) and OpenSSL.  This is very slightly
   // lower-level -- in that fewer things are magically defaulted for you -- and requires us to use std::string
   // rather than QString.  But at least it does not require a callback function.  And, should we have future
   // problems, it should be easier to delve into.
   //
   // Although it's a bit long-winded, we're not really doing anything clever here.  The example code at
   // https://www.boost.org/doc/libs/1_86_0/libs/beast/doc/html/beast/quick_start/http_client.html explains a lot of
   // what's going on.  We're just doing a bit extra to do HTTPS rather than HTTP.
   //
   // Because we're running on a separate thread, we don't worry about doing timeouts etc for the individual parts of
   // the request/response below.  If something takes a long time or we don't get a response at all, no harm is done.
   // The main program carries on running on the main thread and just never receives the foundLatestRelease signal.
   //
   std::string const host{"api.github.com"};
   // It would be neat to construct this string at compile-time, but I haven't yet worked out how!
   std::string const path = QString{"/repos/%1/%2/releases/latest"}.arg(CONFIG_APPLICATION_NAME_UC, CONFIG_APPLICATION_NAME_LC).toStdString();
   std::string const port{"443"};
   //
   // Here 11 means HTTP/1.1, 20 means HTTP/2.0, 30 means HTTP/3.0.  (See
   // https://www.boost.org/doc/libs/1_86_0/libs/beast/doc/html/beast/ref/boost__beast__http__message/version/overload1.html.)
   // If we were doing something generic then we'd stick with HTTP/1.1 since that has 100% support.  But, since we're
   // only making one request, and it's to GitHub, and we know they support they newer version of HTTP, we might as
   // well use the newer standard.
   //
   boost::beast::http::request<boost::beast::http::string_body> httpRequest{boost::beast::http::verb::get,
                                                                            path,
                                                                            30};
   httpRequest.set(boost::beast::http::field::host, host);
   //
   // GitHub will respond with an error if the user agent field is not present, but it doesn't care what it's set to
   // and will even accept empty string.
   //
   httpRequest.set(boost::beast::http::field::user_agent, "");

   std::ostringstream requestAsString;
   requestAsString << "https://" << host << ":" << port << path;
   qInfo() <<
      Q_FUNC_INFO << "Sending request to " << QString::fromStdString(requestAsString.str()) <<
      "to check for latest release";

   try {
///      boost::asio::io_service ioService;
      boost::asio::io_context ioContext;
      //
      // A lot of old example code for Boost still uses sslv23_client.  However, TLS 1.3 has been out since 2018, and we
      // know GitHub (along with most other web sites) supports it.  So there's no reason not to use that.
      //
      boost::asio::ssl::context securityContext(boost::asio::ssl::context::tlsv13_client);
///      boost::asio::ssl::stream<boost::asio::ip::tcp::socket> secureSocket{ioService, securityContext};
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket> secureSocket{ioContext, securityContext};
      // The resolver essentially does the DNS requests to look up the host address etc
///      boost::asio::ip::tcp::resolver tcpIpResolver{ioService};
      boost::asio::ip::tcp::resolver tcpIpResolver{ioContext};
      auto endpoint = tcpIpResolver.resolve(host, port);

      // Once we have the address, we can connect, do the SSL handshake, and then send the request
      boost::asio::connect(secureSocket.lowest_layer(), endpoint);
      secureSocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
      boost::beast::http::write(secureSocket, httpRequest);

      // Now wait for the response
      boost::beast::http::response<boost::beast::http::string_body> httpResponse;
      boost::beast::flat_buffer buffer;
      boost::beast::http::read(secureSocket, buffer, httpResponse);

      if (httpResponse.result() != boost::beast::http::status::ok) {
         //
         // It's not the end of the world if we couldn't check for an update, but we should record the fact.  With some
         // things in Boost.Beast, the easiest way to convert them to a string is via a standard library output stream,
         // so we construct the whole error message like that rather then try to mix-and-match with Qt logging output
         // streams.
         //
         std::ostringstream errorMessage;
         errorMessage <<
            "Error checking for update: " << httpResponse.result_int() << ".\nResponse headers:" << httpResponse.base();
         qInfo().noquote() << Q_FUNC_INFO << QString::fromStdString(errorMessage.str());
         return;
      }

      //
      // Checking a version number on Sourceforge is easy, eg a GET request to
      // https://brewtarget.sourceforge.net/version just returns the last version of Brewtarget that was hosted on
      // Sourceforge (quite an old one).
      //
      // On GitHub, it's a bit harder as there's a REST API that gives back loads of info in JSON format.  We don't want
      // to do anything clever with the JSON response, just extract one field, so the Qt JSON support suffices here.
      // (See comments elsewhere for why we don't use it for BeerJSON.)
      //
      QByteArray rawContent = QByteArray::fromStdString(httpResponse.body());
      QJsonParseError jsonParseError{};

      QJsonDocument jsonDocument = QJsonDocument::fromJson(rawContent, &jsonParseError);
      if (QJsonParseError::ParseError::NoError != jsonParseError.error) {
         qWarning() <<
            Q_FUNC_INFO << "Error parsing JSON from version check response:" << jsonParseError.error << "at offset" <<
            jsonParseError.offset;
         return;

      }

      QJsonObject jsonObject = jsonDocument.object();

      QString remoteVersion = jsonObject.value("tag_name").toString();
      // Version names are usually "v3.0.2" etc, so we want to strip the 'v' off the front
      if (remoteVersion.startsWith("v", Qt::CaseInsensitive)) {
         remoteVersion.remove(0, 1);
      }
      QVersionNumber const latestRelease {QVersionNumber::fromString(remoteVersion)};
      qInfo() <<
         Q_FUNC_INFO << "Latest release is" << remoteVersion << "(parsed as" << latestRelease << ")";
      emit foundLatestRelease(latestRelease);

   } catch (std::exception & e) {
      //
      // Typically we might get an exception if there are network problems.  It's not fatal.  We'll do the check again
      // next time the program is run.
      //
      qWarning() <<
         Q_FUNC_INFO << "Problem while trying to contact GitHub to check for newer version of the software:" <<
         e.what();
   }

   return;
}
