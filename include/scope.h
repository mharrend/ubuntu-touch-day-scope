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

#ifndef DAYSCOPE_H
#define DAYSCOPE_H

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QString>
#include <QList>
#include <QMap>

class Scope : public unity::scopes::ScopeBase
{
public:
    Scope();
    virtual void start(std::string const&) override;
    virtual void run() override;
    virtual void stop() override;

    unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
                                                  const unity::scopes::ActionMetadata&
                                                ) override;

    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
                                                        unity::scopes::SearchMetadata const&) override;

};
#endif
