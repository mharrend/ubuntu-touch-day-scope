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

#ifndef PREVIEW_H
#define PREVIEW_H

#include "scope.h"

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/Result.h>

class Preview : public unity::scopes::PreviewQueryBase
{
public:
    Preview(unity::scopes::Result const&, unity::scopes::ActionMetadata const&);
    ~Preview();
    virtual void cancelled() override;
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) override;
private:
    const unity::scopes::Result result_;
};

#endif
