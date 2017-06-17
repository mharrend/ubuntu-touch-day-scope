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

#include "scope.h"
#include "query.h"
#include "preview.h"
#include "i18n.h"

#include <unity-scopes.h>

#include <QThread>
#include <QDebug>

namespace us = unity::scopes;

Scope::Scope()
{
    int argc = 1;
    char argv[] = "day";
    char * it = &argv[0];
    QCoreApplication *app = new QCoreApplication(argc, &it);
}

void Scope::start(std::string const&)
{
}

void Scope::stop()
{
}

void Scope::run()
{
}

us::SearchQueryBase::UPtr Scope::search(us::CannedQuery const &q,
                                        us::SearchMetadata const &metadata )
{

    const QString scopePath = QString::fromStdString(scope_directory());
    const QString cachePath = QString::fromStdString(cache_directory());

    textdomain(GETTEXT_DOMAIN.toStdString().c_str());

    QString tdir = QString("%1/locale/").arg(scopePath);
    QString basedir = QString(bindtextdomain(GETTEXT_DOMAIN.toStdString().c_str(), tdir.toStdString().c_str()));


    us::SearchQueryBase::UPtr cannedQuery(new Query(q,
                                          metadata,
                                          *this,
                                          cachePath
                                          ));
    return cannedQuery;
}

us::PreviewQueryBase::UPtr Scope::preview(us::Result const& result, us::ActionMetadata const& metadata ) {
    us::PreviewQueryBase::UPtr preview(new Preview(result, metadata));
    return preview;
}

#define EXPORT __attribute__ ((visibility ("default")))

extern "C"
{

    EXPORT
    us::ScopeBase*
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new Scope();
    }

    EXPORT
    void
    UNITY_SCOPE_DESTROY_FUNCTION(us::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
