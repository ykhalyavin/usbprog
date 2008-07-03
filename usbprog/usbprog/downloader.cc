/*
 * (c) 2007-2008, Bernhard Walle <bernhard.walle@gmx.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdexcept>
#include <ostream>
#include <cstring>

#ifdef _WIN32
#  include <wininet.h>
#endif

#include <curl/curl.h>

#include <usbprog/downloader.h>
#include <usbprog/usbprog.h>

using std::string;
using std::ostream;
using std::strlen;

bool Downloader::m_firstCalled = true;
ProxySettings Downloader::m_proxySettings;

#ifdef _WIN32
#  define CURL_GLOBAL_FLAGS   CURL_GLOBAL_WIN32
#else
#  define CURL_GLOBAL_FLAGS   0
#endif

/* Static functions {{{ */

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
#define INFOBUFFER_SIZE 1000
void Downloader::readProxySettings()
{
    // if one of the standard environment variables is set, CURL automatically
    // uses this -- don't overwrite anything here!
    if (getenv("http_proxy") || getenv("ftp_proxy") || getenv("all_proxy"))
        return;

    // read IE settings otherwise
    char buffer[INFOBUFFER_SIZE];
    DWORD buflen = INFOBUFFER_SIZE;

    if (!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, buffer, &buflen))
        return;

	// if a proxy is set
	if (((INTERNET_PROXY_INFO *)buffer)->dwAccessType == INTERNET_OPEN_TYPE_PROXY) {

        string settings = (char *)(((INTERNET_PROXY_INFO *)buffer)->lpszProxy);

        // different proxys for different protocols
        if (settings.find(" ") != string::npos) {
            string::size_type http_proxy = settings.find("http=");
            if (http_proxy == string::npos) {
                Debug::debug()->dbg("No http proxy specified");
                return;
            }

            settings = settings.substr(http_proxy + strlen("http="));
            string::size_type space = settings.find(" ");
            if (space != string::npos)
                settings = settings.substr(0, space);
        }

        m_proxySettings.host = settings;
        Debug::debug()->dbg("IE proxy is set, hostname = %s",
                m_proxySettings.host.c_str());

		buflen = INFOBUFFER_SIZE;
		// try and get a proxy username - ignore a blank result
		if (InternetQueryOption(NULL, INTERNET_OPTION_PROXY_USERNAME,
                    buffer, &buflen)) {

            m_proxySettings.username = (char *)buffer;
            Debug::debug()->dbg("IE proxy username = %s",
                    m_proxySettings.username.c_str());

            buflen = INFOBUFFER_SIZE;
			if (m_proxySettings.username.size() > 0) {
				if (InternetQueryOption(NULL, INTERNET_OPTION_PROXY_PASSWORD,
                            buffer, &buflen)) {

                    m_proxySettings.password = (char *)buffer;
                    Debug::debug()->dbg("IE proxy password = %s",
                            m_proxySettings.password.c_str());
				}
			}
		}
	}
}
#undef INFOBUFFER_SIZE
#else
void Downloader::readProxySettings()
{
    // since CURL uses the environment variables, nothing to do on Unix
    // here
}
#endif


/* -------------------------------------------------------------------------- */
size_t Downloader::curl_write_callback(void *buffer, size_t size,
        size_t nmemb, void *userp)
{
    Downloader *downloader = reinterpret_cast<Downloader *>(userp);

    downloader->m_output.write((char *)buffer, size * nmemb);
    Debug::debug()->dbg("Writing %d*%d=%d bytes (%d)", size, nmemb, size*nmemb,
            int(downloader->m_output.good()));

    if (downloader->m_output.good())
        return size * nmemb;
    else
        return 0;
}


/* -------------------------------------------------------------------------- */
int Downloader::curl_progress_callback(void *clientp, double dltotal, double
        dlnow, double ultotal, double ulnow)
{
    Downloader *downloader = reinterpret_cast<Downloader *>(clientp);

    (void)ultotal;
    (void)ulnow;

    if (downloader->m_notifier)
        downloader->m_notifier->progressed(dltotal, dlnow);

    return 0;
}

/* }}} */
/* Members {{{ */

/* -------------------------------------------------------------------------- */
Downloader::Downloader(ostream &output) throw (DownloadError)
    : m_notifier(NULL), m_output(output)
{
    CURLcode err;

    // perform CURL initialisation only once
    if (m_firstCalled) {
        Debug::debug()->dbg("Initialize CURL");
        curl_global_init(CURL_GLOBAL_FLAGS);
        readProxySettings();
        m_firstCalled = false;
    }

    m_curl = curl_easy_init();
    if (!m_curl)
        throw DownloadError("curl_easy_init returned NULL");

    // error buffer
    err = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_curl_errorstring);
    if (err != CURLE_OK)
        throw DownloadError("CURLOPT_ERRORBUFFER failed");

    // write function
    err = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,
            Downloader::curl_write_callback);
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);

    // write data
    err = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);

    // set proxy settings
    if (m_proxySettings.host.size() > 0) {
		err = curl_easy_setopt(m_curl, CURLOPT_PROXY, m_proxySettings.host.c_str());
        Debug::debug()->dbg("Setting CURL proxy to %s",
                m_proxySettings.host.c_str());
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);

        if (m_proxySettings.username.size() > 0) {
            string str = m_proxySettings.username;
            if (m_proxySettings.password.size() > 0)
                str += ":" + m_proxySettings.password;

            err = curl_easy_setopt(m_curl,  CURLOPT_PROXYUSERPWD, str.c_str());
            Debug::debug()->dbg("Setting CURL username/password for proxy to %s",
                    str.c_str());
            if (err != CURLE_OK)
                throw DownloadError(string("CURL error: ") + m_curl_errorstring);
        }
    }
}

/* -------------------------------------------------------------------------- */
Downloader::~Downloader()
{
    if (m_curl)
        curl_easy_cleanup(m_curl);
}

/* -------------------------------------------------------------------------- */
void Downloader::setUrl(const string &url) throw (DownloadError)
{
    CURLcode err;

    m_url = url;

    Debug::debug()->dbg("Setting URL to %s", m_url.c_str());
    err = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);
}

/* -------------------------------------------------------------------------- */
string Downloader::getUrl() const
{
    return m_url;
}

/* -------------------------------------------------------------------------- */
void Downloader::setProgress(ProgressNotifier *notifier)
{
    CURLcode err;
    m_notifier = notifier;

    if (notifier) {
        err = curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION,
                Downloader::curl_progress_callback);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);

        err = curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);

        err = curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, false);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);
    } else {
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, NULL);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, NULL);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, true);
    }
}

/* -------------------------------------------------------------------------- */
void Downloader::download() throw (DownloadError)
{
    CURLcode err;

    Debug::debug()->dbg("Performing download");
    err = curl_easy_perform(m_curl);
    if (m_notifier)
        m_notifier->finished();
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
