// Compile from windows/: cl /std:c++17 /EHsc /utf-8 tests\tuxedo-preview.cpp
//   /Fe:tuxedo-preview.exe /link gdiplus.lib user32.lib advapi32.lib gdi32.lib
// Runs offline; no global input registration or registry writes.
#include "../src/CatRenderer.hpp"
#include <iostream>

int wmain() {
    using namespace Gdiplus;
    GdiplusStartupInput init;
    ULONG_PTR token=0;
    if (GdiplusStartup(&token,&init,nullptr)!=Ok) return 1;
    int result=0;
    {
        Bitmap sheet(2160,640,PixelFormat32bppARGB);
        Graphics g(&sheet);g.Clear(Color(255,250,249,246));
        CatSettings settings;
        settings.coatTheme=CatCoatTheme::SlenderTuxedo;
        settings.eyeTheme=EyeColorTheme::SageGreen;
        const wchar_t* labels[]={L"Idle",L"Look left/up",L"Look right/down",L"Hunting",L"Typing + third paw",L"Petting"};
        for (int i=0;i<6;++i) {
            CatPhysics p;
            if (i==1 || i==2) {
                float sign=i==1 ? -1.0f : 1.0f;
                p.headYaw.value=sign;p.headPitch.value=sign;p.headTilt.value=sign*0.12f;
                p.eyeGaze.x.value=sign*0.8f;p.eyeGaze.y.value=sign*0.8f;
            }
            if (i==3) {p.huntIntensity.value=1;p.bodySquash.value=.89f;p.headOffsetY.value=10.5f;p.earFold.value=-.18f;}
            if (i==4) {
                p.leftPawTyping=true;p.rightPawTyping=true;p.hindFootActive.value=1;
                Pt a=KeyboardLayout::canvasPointForKey(PhysicalKey::J);
                Pt b=KeyboardLayout::canvasPointForKey(PhysicalKey::W);
                Pt c=KeyboardLayout::canvasPointForKey(PhysicalKey::Space);
                p.leftPaw.x.value=a.x;p.leftPaw.y.value=a.y;
                p.rightPaw.x.value=b.x;p.rightPaw.y.value=b.y;
                p.hindFoot.x.value=c.x;p.hindFoot.y.value=c.y;
                p.keyGlow[PhysicalKey::J]=p.keyGlow[PhysicalKey::W]=p.keyGlow[PhysicalKey::Space]=1;
            }
            if (i==5) {p.petHeadIntensity=1;p.earFold.value=.7f;}
            GraphicsState state=g.Save();
            g.TranslateTransform(float((i%3)*720),float((i/3)*320));
            g.TranslateTransform(180.0f,0.0f);
            CatRenderer::draw(g,p,settings);
            Font font(L"Segoe UI",12);SolidBrush ink(Color(255,40,40,40));
            g.DrawString(labels[i],-1,&font,PointF(100,302),&ink);
            g.Restore(state);
        }
        g.Flush(FlushIntentionSync);
        // Check the neutral frame's actual output, not a separate mock renderer.
        Color fur,eye,nose;
        sheet.GetPixel(364,90,&fur);
        sheet.GetPixel(345,110,&eye);
        sheet.GetPixel(364,127,&nose);
        if (fur.GetR()>60 || eye.GetG()<100 || nose.GetR()<=nose.GetG()) {
            std::cerr << "Neutral skin pixel checks failed\n";result=2;
        }
        CatPhysics neutral;
        if (CatRenderer::hitTest({184,112},neutral,settings.coatTheme)!=CatHitZone::Head ||
            CatRenderer::hitTest({184,190},neutral,settings.coatTheme)!=CatHitZone::Body ||
            CatRenderer::hitTest({230,190},neutral,settings.coatTheme)!=CatHitZone::None) {
            std::cerr << "Skin hit testing failed\n";result=3;
        }
        const CLSID png={0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
        if (sheet.Save(L"tuxedo-native-preview.png",&png,nullptr)!=Ok) result=4;
    }
    GdiplusShutdown(token);
    return result;
}
