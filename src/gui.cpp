#define GUI_HEAVY
//#define MUSIC

#ifdef MUSIC
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_openmpt.h>
const unsigned char song_src[] = {
    #embed "asset/ParisClub2_Music.it"
};

SoLoud::Soloud soloud;
SoLoud::Openmpt song;
#endif


#ifdef GUI_HEAVY
#include "wx/log.h"
#include <filesystem>
#include <wx/wx.h>
#include <zlib-ng.h>
#include <juggler.hpp>

const unsigned char logo_src_png[] = {
    #embed "asset/logo.png"
};

class App : public wxApp
{
    public:
        bool OnInit() override;
};

class PanelMain : public wxPanel
{
    wxBitmap *logo;

    public:
        PanelMain(wxFrame *parent);

        void paintEvent(wxPaintEvent &event);
        void paintNow();

        void render(wxDC& dc);

        DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(PanelMain, wxPanel)

EVT_PAINT(PanelMain::paintEvent)

END_EVENT_TABLE()

PanelMain::PanelMain(wxFrame *parent) : wxPanel(parent)
{
    logo = new wxBitmap{wxBITMAP_PNG_FROM_DATA(logo_src)};
};

void PanelMain::paintEvent(wxPaintEvent &event)
{
    wxPaintDC dc(this);
    render(dc);
}

void PanelMain::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}

void PanelMain::render(wxDC &dc)
{
    dc.DrawBitmap(*logo, wxPoint(0, 0));
}

enum
{
    BUTTON_CREATE   = 2,
    BUTTON_APPLY    = 3,
};

class FrameMain : public wxFrame
{
    public:
        FrameMain();
    private:
        PanelMain *panel = new PanelMain(this);
        wxButton *button_create = new wxButton(panel, BUTTON_CREATE, "create patch", wxPoint(16,16), wxSize(84, 24));
        wxButton *button_apply = new wxButton(panel, BUTTON_APPLY, "apply patch", wxPoint(16,44), wxSize(84, 24));

        void OnExit(wxCommandEvent &event);

        void OnCreateClicked(wxCommandEvent &event);
        void OnApplyClicked(wxCommandEvent &event);

        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(FrameMain, wxFrame)
    EVT_BUTTON(BUTTON_CREATE, FrameMain::OnCreateClicked)
    EVT_BUTTON(BUTTON_APPLY, FrameMain::OnApplyClicked)
wxEND_EVENT_TABLE()

#define CHECK_RESULT() if (result != ERROR_OK) \
    { \
        switch (result) \
        { \
            case ERROR_FILE1_ACCESS: \
                wxLogError("ERROR_FILE1_ACCESS"); \
                break; \
            case ERROR_FILE2_ACCESS: \
                wxLogError("ERROR_FILE2_ACCESS"); \
                break; \
            case ERROR_FILE3_ACCESS: \
                wxLogError("ERROR_FILE3_ACCESS"); \
                break; \
            case ERROR_WRONG_HASH: \
                wxLogError("ERROR_WRONG_HASH"); \
                break; \
            case ERROR_SAME_HASH: \
                wxLogError("ERROR_SAME_HASH"); \
                break; \
            case ERROR_INVALID_FILE: \
                wxLogError("ERROR_INVALID_FILE"); \
                break; \
            case ERROR_WRONG_OUTPUT_HASH: \
                wxLogError("ERROR_WRONG_OUTPUT_HASH"); \
                break; \
        } \
    } \

void FrameMain::OnCreateClicked(wxCommandEvent &event)
{
    wxFileDialog FileDialogOriginal(this, _("open original file"), "", "", "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (FileDialogOriginal.ShowModal() == wxID_CANCEL)
        return;
    std::string file_original(FileDialogOriginal.GetPath().ToStdString());

    wxFileDialog FileDialogModified(this, _("open modified file"), "", "", "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (FileDialogModified.ShowModal() == wxID_CANCEL)
        return;
    std::string file_modified(FileDialogModified.GetPath().ToStdString());

    wxFileDialog FileDialogPatch(this, _("patch destination"), "", "", "DXP files (*.dxp)|*.dxp", wxFD_SAVE);
    if (FileDialogPatch.ShowModal() == wxID_CANCEL)
        return;
    std::string file_patch(FileDialogPatch.GetPath().ToStdString());
    std::filesystem::path fs(file_patch);

    if (std::filesystem::exists(file_patch))
    {
        std::string message = "file " + fs.filename().string() + " already exists.\n overwrite?";
        wxMessageDialog DialogConfirmOverwrite(this, message, "confirm", wxOK|wxCANCEL|wxCENTRE);
        switch (DialogConfirmOverwrite.ShowModal())
        {
            case wxID_OK:
                break;
            case wxID_CANCEL:
                return;
        }
    }
    int result = create_patch(file_original, file_modified, file_patch);
    CHECK_RESULT()
    else
        wxLogMessage("patch made successfully");

}
void FrameMain::OnApplyClicked(wxCommandEvent &event)
{
    wxFileDialog FileDialogOriginal(this, _("open original file"), "", "", "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (FileDialogOriginal.ShowModal() == wxID_CANCEL)
        return;
    std::string file_original(FileDialogOriginal.GetPath().ToStdString());

    wxFileDialog FileDialogPatch(this, _("open patch"), "", "", "DXP files (*.dxp)|*.dxp", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (FileDialogPatch.ShowModal() == wxID_CANCEL)
        return;
    std::string file_patch(FileDialogPatch.GetPath().ToStdString());

    wxFileDialog FileDialogModified(this, _("modified file destination"), "", "", "", wxFD_SAVE);
    if (FileDialogModified.ShowModal() == wxID_CANCEL)
        return;
    std::string file_modified(FileDialogModified.GetPath().ToStdString());
    std::filesystem::path fs(file_modified);

    if (std::filesystem::exists(file_modified))
    {
        std::string message = "file " + fs.filename().string() + " already exists.\n overwrite?";
        wxMessageDialog DialogConfirmOverwrite(this, message, "confirm", wxOK|wxCANCEL|wxCENTRE);
        switch (DialogConfirmOverwrite.ShowModal())
        {
            case wxID_OK:
                break;
            case wxID_CANCEL:
                return;
        }
    }
    int result = apply_patch(file_original, file_patch, file_modified);
    CHECK_RESULT()
    else
        wxLogMessage("patch applied successfully");
}

bool App::OnInit()
{
    wxInitAllImageHandlers();
    FrameMain *main_frame = new FrameMain();
    main_frame->Show();
    main_frame->SetSize(wxSize(250,180));
    main_frame->Center();
    main_frame->SetWindowStyle(wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX));
    #ifdef MUSIC
    soloud.init();

    song.loadMem(song_src, sizeof(song_src) / sizeof(unsigned char), false, false);
    soloud.play(song);
    #endif

    return true;
}

FrameMain::FrameMain() : wxFrame(nullptr, wxID_ANY, "Juggler")
{
    #ifdef _WIN32
        wxIcon icon("CLOWN");
        SetIcon(icon);
    #endif
    SetBackgroundColour(wxColour(245, 233, 217));

    Bind(wxEVT_MENU, &FrameMain::OnExit, this, wxID_EXIT);
}

void FrameMain::OnExit(wxCommandEvent& event)
{
    this->HideWithEffect(wxSHOW_EFFECT_EXPAND);
}

wxIMPLEMENT_APP(App);
#else
#error NO INTERFACE DEFINED
#endif