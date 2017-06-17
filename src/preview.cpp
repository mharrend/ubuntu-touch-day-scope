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

#include "preview.h"
#include "i18n.h"

#include<unity/scopes/PreviewWidget.h>
#include<unity/scopes/ColumnLayout.h>
#include<unity/scopes/PreviewReply.h>
#include <unity/scopes/VariantBuilder.h>
#include <unity/scopes/Variant.h>
#include <unity-scopes.h>

#include <QDebug>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

using namespace unity::scopes;

Preview::Preview(Result const& result, ActionMetadata const& metadata) :
    PreviewQueryBase(result, metadata),
    result_(result)
{
}

Preview::~Preview()
{
}

void Preview::cancelled()
{
}

void Preview::run(unity::scopes::PreviewReplyProxy const& reply)
{

    ColumnLayout layout1col(1);
    std::vector<std::string> ids = {"headerId", "text1", "actionsId"};

    ColumnLayout layout2col(2);
    layout2col.add_column({"headerId", "text1", "actionsId"});
    layout2col.add_column({});

    ColumnLayout layout3col(3);
    layout3col.add_column({"headerId", "text1", "actionsId"});
    layout3col.add_column({});
    layout3col.add_column({});

    PreviewWidgetList widgets;
/*
    PreviewWidget w_art("artId", "image");
    w_art.add_attribute_mapping("source", "art");
    w_art.add_attribute_value("zoomable", Variant(true));
*/
    PreviewWidget w_header("headerId", "header");
    w_header.add_attribute_mapping("title", "title");
    w_header.add_attribute_mapping("subtitle", "header_text");

    PreviewWidget w_text1("text1", "text");
    w_text1.add_attribute_mapping("text", "summary");

    widgets.emplace_back(w_header);
    widgets.emplace_back(w_text1);

    PreviewWidget w_actions("actionsId", "actions");
    VariantBuilder builder;
    builder.add_tuple({
        {"id", Variant("open")},
        {"label", Variant("Open")}// not i18n because the preview never shows
    });
    w_actions.add_attribute_value("actions", builder.end());

    widgets.emplace_back(w_actions);

    layout1col.add_column(ids);
    reply->register_layout({layout1col, layout2col, layout3col});
    reply->push(widgets);
}
