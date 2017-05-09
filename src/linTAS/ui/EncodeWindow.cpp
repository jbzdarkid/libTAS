/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EncodeWindow.h"
#include "MainWindow.h"
#include <iostream>

EncodeWindow::EncodeWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 200);

    /* Game Executable */
    encodepath = new Fl_Output(10, 30, 400, 30, "Encode file path");
    encodepath->align(FL_ALIGN_TOP_LEFT);
    encodepath->color(FL_LIGHT1);

    browseencodepath = new Fl_Button(520, 30, 70, 30, "Browse...");
    browseencodepath->callback(browse_encodepath_cb, this);

    encodepathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    encodepathchooser->title("Choose an encode filename");
    encodepathchooser->options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);

    containerchoice = new Fl_Choice(420, 30, 80, 30);
    containerchoice->menu(container_items);

    videochoice = new Fl_Choice(10, 100, 100, 30, "Video codec");
    videochoice->align(FL_ALIGN_TOP_LEFT);
    videochoice->menu(video_items);
    videochoice->callback(vcodec_cb, this);

    videobitrate = new Fl_Input(10, 160, 100, 30, "Video bitrate");
    videobitrate->align(FL_ALIGN_TOP_LEFT);

    audiochoice = new Fl_Choice(150, 100, 100, 30, "Audio codec");
    audiochoice->align(FL_ALIGN_TOP_LEFT);
    audiochoice->menu(audio_items);
    audiochoice->callback(acodec_cb, this);

    audiobitrate = new Fl_Input(150, 160, 100, 30, "Audio bitrate");
    audiobitrate->align(FL_ALIGN_TOP_LEFT);

    start = new Fl_Button(400, 160, 70, 30, "Ok");
    start->callback(start_cb, this);

    cancel = new Fl_Button(500, 160, 70, 30, "Cancel");
    cancel->callback(cancel_cb, this);

    update_config();
    window->end();
}

Fl_Menu_Item EncodeWindow::container_items[] = {
    {".mkv"},
    {".avi"},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::video_items[] = {
    {"h264", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_H264)},
    {"ffv1", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_FFV1)},
    {"raw", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_RAWVIDEO)},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::audio_items[] = {
    {"vorbis", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_VORBIS)},
    {"opus", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_OPUS)},
    {"flac", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_FLAC)},
    {"pcm", 0, nullptr, reinterpret_cast<void*>(AV_CODEC_ID_PCM_S16LE)},
    {nullptr}
};

void EncodeWindow::update_config()
{
    if (context->config.dumpfile.empty()) {
        encodepath->value(context->gamepath.c_str());
    }
    else {
        encodepath->value(context->config.dumpfile.c_str());
    }
    encodepathchooser->preset_file(encodepath->value());

    /* Browse the list of video codecs and select the item that matches
     * the value in the config using the item's user data.
     */
    const Fl_Menu_Item* vcodec_item = videochoice->menu();
    // vcodec_item = vcodec_item->first();
    while (vcodec_item) {
        if (static_cast<AVCodecID>(vcodec_item->argument()) == context->config.sc.video_codec) {
            videochoice->value(vcodec_item);
            break;
        }
        vcodec_item = vcodec_item->next();
    }

    /* Enable/disable video bitrate */
    switch(context->config.sc.video_codec) {
        case AV_CODEC_ID_FFV1:
        case AV_CODEC_ID_RAWVIDEO:
            videobitrate->deactivate();
            break;
        case AV_CODEC_ID_H264:
        default:
            videobitrate->activate();
    }

    /* Set video bitrate */
    std::string vbstr = std::to_string(context->config.sc.video_bitrate);
    videobitrate->value(vbstr.c_str());

    /* Same for audio codec and bitrate */
    const Fl_Menu_Item* acodec_item = audiochoice->menu();
    acodec_item = acodec_item->first();
    while (acodec_item) {
        if (static_cast<AVCodecID>(acodec_item->argument()) == context->config.sc.audio_codec) {
            audiochoice->value(acodec_item);
            break;
        }
        acodec_item = acodec_item->next();
    }

    /* Enable/disable audio bitrate */
    switch(context->config.sc.audio_codec) {
        case AV_CODEC_ID_FLAC:
        case AV_CODEC_ID_PCM_S16LE:
            audiobitrate->deactivate();
            break;
        case AV_CODEC_ID_VORBIS:
        case AV_CODEC_ID_OPUS:
        default:
            audiobitrate->activate();
    }

    std::string abstr = std::to_string(context->config.sc.audio_bitrate);
    audiobitrate->value(abstr.c_str());
}

void start_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Fill encode filename */
    const char* filename = ew->encodepath->value();
    std::string ext = ew->containerchoice->mvalue()->label();

    /* Check if the extension is already in the filename. If not, append it. */
    std::string filenameext = filename;
    size_t sep = filenameext.find_last_of(".");
    if ((sep != std::string::npos) && (filenameext.compare(sep, std::string::npos, ext) == 0)) {
        ew->context->config.dumpfile = filename;
    }
    else {
        ew->context->config.dumpfile = std::string(filename) + ext;
    }

    /* Set video codec and bitrate */
    const Fl_Menu_Item* vcodec_item = ew->videochoice->mvalue();
    ew->context->config.sc.video_codec = static_cast<AVCodecID>(vcodec_item->argument());
    ew->context->config.sc.video_bitrate = std::stoi(ew->videobitrate->value());

    /* Set audio codec and bitrate */
    const Fl_Menu_Item* acodec_item = ew->audiochoice->mvalue();
    ew->context->config.sc.audio_codec = static_cast<AVCodecID>(acodec_item->argument());
    ew->context->config.sc.audio_bitrate = std::stoi(ew->audiobitrate->value());

    ew->context->config.sc_modified = true;

    /* Close window */
    ew->window->hide();
}

void cancel_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Close window */
    ew->window->hide();
}

void browse_encodepath_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    int ret = ew->encodepathchooser->show();

    const char* filename = ew->encodepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        ew->encodepath->value(filename);
    }
}

void vcodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    const Fl_Menu_Item* vcodec_item = ew->videochoice->mvalue();

    /* Enable/disable video bitrate */
    switch(vcodec_item->argument()) {
        case AV_CODEC_ID_FFV1:
        case AV_CODEC_ID_RAWVIDEO:
            ew->videobitrate->deactivate();
            break;
        case AV_CODEC_ID_H264:
        default:
            ew->videobitrate->activate();
    }
}

void acodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    const Fl_Menu_Item* acodec_item = ew->audiochoice->mvalue();

    /* Enable/disable audio bitrate */
    switch(acodec_item->argument()) {
        case AV_CODEC_ID_FLAC:
        case AV_CODEC_ID_PCM_S16LE:
            ew->audiobitrate->deactivate();
            break;
        case AV_CODEC_ID_VORBIS:
        case AV_CODEC_ID_OPUS:
        default:
            ew->audiobitrate->activate();
    }

    if (acodec_item->argument() == AV_CODEC_ID_OPUS) {
        /* For Opus codec, only some frequencies are supported */
        int freq = ew->context->config.sc.audio_frequency;
        if ((freq % 4000) || (freq == 32000)) {
            /* If we are not runnning, change the frequency */
            if (ew->context->status == Context::INACTIVE) {
                fl_alert("The sound frequency %d Hz is not compatible with Opus. It is changed to 48000 Hz", freq);
                ew->context->config.sc.audio_frequency = 48000;
                MainWindow& mw = MainWindow::getInstance();
                Fl_Menu_Item* freq_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item("Sound/Format/48000 Hz"));
                if (freq_item) freq_item->setonly();
            }
            else {
                fl_alert("The sound frequency %d Hz is not compatible with Opus. You must quit the game and then change the frequency", freq);
                ew->audiochoice->value(0);
            }
        }
    }
}