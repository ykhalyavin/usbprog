/*
 * (c) 2007, Bernhard Walle <bernhard.walle@gmx.de>
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
#ifndef USBPROG_DOWNLOADER_H
#define USBPROG_DOWNLOADER_H

#include <stdexcept>
#include <ostream>

#include <usbprog/usbprog.h>
#include <curl/curl.h>

/* DownloadError {{{1 */

class DownloadError : public std::runtime_error {
    public:
        DownloadError(const std::string& string)
            : std::runtime_error(string) {}
};

/* Downloader {{{1 */

class Downloader {
    public:
        Downloader(std::ostream &output) throw (DownloadError);
        virtual ~Downloader();

    public:
        void setUrl(const std::string &url) throw (DownloadError);
        std::string getUrl() const;

        void setProgress(ProgressNotifier *notifier);
        void download() throw (DownloadError);


    protected:
        static int curl_progress_callback(void *clientp, double dltotal,
                double dlnow, double ultotal, double ulnow);
        static size_t curl_write_callback(void *buffer, size_t size,
                size_t nmemb, void *userp);

    private:
        ProgressNotifier  *m_notifier;
        std::string       m_url;
        CURL              *m_curl;
        char              m_curl_errorstring[CURL_ERROR_SIZE];
        std::ostream      &m_output;
        static bool       m_firstCalled;
};


#endif /* USBPROG_DOWNLOADER_H */

// vim: set sw=4 ts=4 fdm=marker et:
