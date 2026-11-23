#pragma once
#define NOMINMAX
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include "CatPhysics.hpp"
#include "../../shared/CoatPatterns.hpp"

enum class CatHitZone {
    None,
    Head,
    Body
};

class CatRenderer {
public:
    static constexpr float RadToDeg = 57.2957795f;

    static CatHitZone hitTest(Pt p, const CatPhysics& physics, CatCoatTheme coat = CatCoatTheme::ClassicBlack) {
        float cx = CatPhysics::SeatAnchor.x + physics.bodyShiftX.value;
        float breath = std::sin(physics.time * 2.1f) * 1.6f;
        Pt headCenter = { cx + physics.bodyTilt.value * 36.0f + physics.headYaw.value * 7.5f,
                          126.0f + physics.headOffsetY.value + physics.headPitch.value * 4.8f + breath * 1.1f };

        if (coat == CatCoatTheme::SlenderTuxedo) {
            headCenter.y -= 14.0f;
            float dx=p.x-headCenter.x, dy=p.y-headCenter.y;
            float c=std::cos(physics.headTilt.value), s=std::sin(physics.headTilt.value);
            float hx=(dx*c+dy*s)/39.0f, hy=(-dx*s+dy*c)/34.0f;
            if (hx*hx+hy*hy <= 1.0f) return CatHitZone::Head;
            float sy=physics.bodySquash.value, sx=1+(1-sy)*0.55f;
            dx=p.x-cx; dy=p.y-CatPhysics::SeatAnchor.y;
            c=std::cos(physics.bodyTilt.value); s=std::sin(physics.bodyTilt.value);
            float bx=(dx*c+dy*s)/(32.0f*sx), by=((-dx*s+dy*c)/sy+34.0f)/51.0f;
            if (bx*bx+by*by <= 1.0f) return CatHitZone::Body;
            if (p.x >= 58 && p.x <= 294 && p.y >= 232 && p.y <= 296) return CatHitZone::Body;
            return CatHitZone::None;
        }
        float hdx = (p.x - headCenter.x) / 48.0f;
        float hdy = (p.y - headCenter.y) / 42.0f;
        if (hdx * hdx + hdy * hdy <= 1.0f) return CatHitZone::Head;

        float bdx = (p.x - cx) / 58.0f;
        float bdy = (p.y - 196.0f) / 56.0f;
        if (bdx * bdx + bdy * bdy <= 1.0f) return CatHitZone::Body;

        if (p.x >= 58.0f && p.x <= 294.0f && p.y >= 232.0f && p.y <= 296.0f) return CatHitZone::Body;
        return CatHitZone::None;
    }

    static void draw(Gdiplus::Graphics& g, const CatPhysics& physics, const CatSettings& settings) {
        using namespace Gdiplus;
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        float breath = std::sin(physics.time * 2.1f) * 1.6f;
        float seatX = CatPhysics::SeatAnchor.x + physics.bodyShiftX.value;
        float seatY = CatPhysics::SeatAnchor.y;

        Pt headCenter = {
            seatX + physics.bodyTilt.value * 36.0f + physics.headYaw.value * 7.5f,
            126.0f + physics.headOffsetY.value + physics.headPitch.value * 4.8f + breath * 1.1f
        };

        // 1. Mini White Mouse (facing cat)
        drawMouse(g, physics, settings.keyboardTheme);

        // 2. Mini 65% Mechanical Keyboard (facing cat) + Backlit Glow
        drawKeyboard(g, physics, settings.keyboardTheme, settings.eyeTheme);

        if (settings.coatTheme == CatCoatTheme::SlenderTuxedo) {
            headCenter.y -= 14.0f;
            drawSlenderTuxedo(g, seatX, seatY, headCenter, physics, settings);
            if (settings.showKeyBadges) drawFloatingKeycaps(g, physics);
            drawSparkles(g, physics, settings.eyeTheme);
            return;
        }

        // 3. Tail
        drawTail(g, seatX, seatY, physics, settings.coatTheme);

        // 4. 3D-Projected Ears (behind crown silhouette)
        drawEarsLayer(g, headCenter, physics, settings.coatTheme);

        // 5. Unified C1-Continuous Silhouette (+ 2.5D Siamese Mask)
        drawUnifiedSilhouette(g, seatX, seatY, headCenter, breath, physics, settings.coatTheme);

        // 6. Hind Foot (3rd simultaneous key easter egg!)
        if (physics.hindFootActive.value > 0.03f) {
            drawHindFoot(g, seatX, seatY, physics, settings.coatTheme);
        }

        // 7. 2.5D Eyes & Whiskers
        drawHeadFeatures(g, headCenter, physics, settings.eyeTheme, settings.coatTheme);

        // 8. Front Paws (Idle Sitting & Dynamic Reaching)
        drawFrontPaws(g, seatX, seatY, breath, physics, settings.coatTheme);

        // 9. Floating Keycap Badges
        if (settings.showKeyBadges) {
            drawFloatingKeycaps(g, physics);
        }

        // 10. Subtle Petting Sparkles & Sleep Z
        drawSparkles(g, physics, settings.eyeTheme);
    }

private:
    static Gdiplus::Color catBlack(BYTE a = 255)      { return Gdiplus::Color(a, 22, 22, 24); }
    static Gdiplus::Color contourMint(BYTE a = 194)   { return Gdiplus::Color(a, 209, 235, 227); }
    static Gdiplus::Color whiskerWhite(BYTE a = 174)  { return Gdiplus::Color(a, 224, 230, 235); }
    static Gdiplus::Color siameseCream(BYTE a = 255)  { return Gdiplus::Color(a, 241, 231, 213); }
    static Gdiplus::Color siameseLatte(BYTE a = 255)  { return Gdiplus::Color(a, 221, 203, 177); }
    static Gdiplus::Color siameseMocha(BYTE a = 255)  { return Gdiplus::Color(a, 115, 87, 69); }
    static Gdiplus::Color siamesePoint(BYTE a = 255)  { return Gdiplus::Color(a, 45, 33, 27); }
    static Gdiplus::Color siameseOutline(BYTE a = 224){ return Gdiplus::Color(a, 66, 48, 40); }
    static Gdiplus::Color siameseWhisker(BYTE a = 174){ return Gdiplus::Color(a, 92, 69, 56); }

    static bool patterned(CatCoatTheme c) { return c == CatCoatTheme::Shorthair || c == CatCoatTheme::OrangeWhite; }
    static Gdiplus::Color rgb(unsigned int hex, BYTE a = 255) { return Gdiplus::Color(a, (hex >> 16) & 255, (hex >> 8) & 255, hex & 255); }
    static Gdiplus::Color bodyColor(CatCoatTheme c) { return c == CatCoatTheme::Shorthair ? rgb(0xC2C5C1) : (c == CatCoatTheme::OrangeWhite ? rgb(0xF2EEE4) : catBlack()); }
    static Gdiplus::Color limbColor(CatCoatTheme c) { return c == CatCoatTheme::OrangeWhite ? rgb(0xD8AA73) : bodyColor(c); }
    static Gdiplus::Color pawColor(CatCoatTheme c) { return (patterned(c) || c == CatCoatTheme::SlenderTuxedo) ? rgb(0xF2EEE4) : (c == CatCoatTheme::Siamese ? siamesePoint() : catBlack()); }
    static Gdiplus::Color earTailColor(CatCoatTheme c) { return c == CatCoatTheme::Siamese ? siamesePoint() : limbColor(c); }
    static Gdiplus::Color creaseColor(CatCoatTheme c, BYTE a = 255) { return (patterned(c) || c == CatCoatTheme::SlenderTuxedo) ? rgb(0x8B8171, (BYTE)(a*0.60f)) : contourMint(a); }
    static Gdiplus::PointF coatMarkPoint(CoatPoint p, bool headSurface, bool skull, CatCoatTheme coat, float seatX, float seatY, Pt head, const CatPhysics& physics) {
        if (headSurface) {
            float depth=coat==CatCoatTheme::OrangeWhite ? 29.6f*std::sqrt(std::max(0.0f,1-p.x*p.x/(46*46)-p.y*p.y/(40*40))) : 27.5f;
            Pt q=skull ? Pt{p.x,p.y} : projectHead3D(p.x,p.y,depth,physics.headYaw.value*0.50f,physics.headPitch.value*0.36f);
            float c = std::cos(physics.headTilt.value), s = std::sin(physics.headTilt.value);
            return {head.x + q.x*c - q.y*s, head.y + q.x*s + q.y*c};
        }
        float sy = physics.bodySquash.value, sx = 1.0f+(1.0f-sy)*0.55f;
        float c = std::cos(physics.bodyTilt.value), s = std::sin(physics.bodyTilt.value);
        return {seatX+p.x*sx*c-p.y*sy*s, seatY+p.x*sx*s+p.y*sy*c};
    }
    static void drawCoatMarks(Gdiplus::Graphics& g, float seatX, float seatY, Pt head, const CatPhysics& physics, CatCoatTheme coat) {
        using namespace Gdiplus;
        const auto& marks = coat == CatCoatTheme::Shorthair ? shorthairMarks() : orangeWhiteMarks();
        for (const auto& mark : marks) {
            std::vector<PointF> points;
            for (const auto& p : mark.points) points.push_back(coatMarkPoint(p, mark.head, mark.skull, coat, seatX, seatY, head, physics));
            GraphicsPath path;
            Color color = rgb(mark.rgb, (BYTE)(mark.alpha*255));
            if (mark.width == 0) { path.AddPolygon(points.data(), (INT)points.size()); SolidBrush brush(color); g.FillPath(&brush, &path); }
            else { path.AddLines(points.data(), (INT)points.size()); Pen pen(color, mark.width); pen.SetStartCap(LineCapRound); pen.SetEndCap(LineCapRound); pen.SetLineJoin(LineJoinRound); g.DrawPath(&pen, &path); }
        }
    }

    static Gdiplus::Color eyePrimary(EyeColorTheme t, BYTE a = 255) {
        switch (t) {
            case EyeColorTheme::SageGreen: return Gdiplus::Color(a, 166, 184, 144);
            case EyeColorTheme::NeonChartreuse: return Gdiplus::Color(a, 201, 242, 64);
            case EyeColorTheme::SiameseBlue:    return Gdiplus::Color(a, 94, 199, 250);
            case EyeColorTheme::WarmAmber:      return Gdiplus::Color(a, 250, 194, 56);
            case EyeColorTheme::IceCyan:        return Gdiplus::Color(a, 115, 235, 245);
            case EyeColorTheme::ReferenceHazel: return Gdiplus::Color(a, 218, 178, 87);
            case EyeColorTheme::SoftAmber: return Gdiplus::Color(a, 217, 173, 85);
        }
        return Gdiplus::Color(a, 201, 242, 64);
    }

    static Gdiplus::Color blendColor(Gdiplus::Color a, Gdiplus::Color b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        auto lerp = [t](BYTE x, BYTE y) -> BYTE { return (BYTE)std::clamp((int)(x + (y - x) * t + 0.5f), 0, 255); };
        return Gdiplus::Color(lerp(a.GetA(), b.GetA()), lerp(a.GetR(), b.GetR()), lerp(a.GetG(), b.GetG()), lerp(a.GetB(), b.GetB()));
    }

    static void addRoundedRect(Gdiplus::GraphicsPath& p, float x, float y, float w, float h, float r) {
        float d = std::min(r * 2.0f, std::min(w, h));
        if (d <= 0.5f) {
            p.AddRectangle(Gdiplus::RectF(x, y, w, h));
            return;
        }
        p.AddArc(x, y, d, d, 180.0f, 90.0f);
        p.AddArc(x + w - d, y, d, d, 270.0f, 90.0f);
        p.AddArc(x + w - d, y + h - d, d, d, 0.0f, 90.0f);
        p.AddArc(x, y + h - d, d, d, 90.0f, 90.0f);
        p.CloseFigure();
    }

    static void addQuadPoly(Gdiplus::GraphicsPath& p, Pt tl, Pt tr, Pt br, Pt bl) {
        Gdiplus::PointF pts[4] = { {tl.x, tl.y}, {tr.x, tr.y}, {br.x, br.y}, {bl.x, bl.y} };
        p.AddPolygon(pts, 4);
    }

    static void addQuadBezier(Gdiplus::GraphicsPath& p, Pt p0, Pt ctrl, Pt p2) {
        // Convert quadratic Bezier (p0, ctrl, p2) -> exact cubic Bezier for GDI+
        Gdiplus::PointF c1(p0.x + (2.0f / 3.0f) * (ctrl.x - p0.x), p0.y + (2.0f / 3.0f) * (ctrl.y - p0.y));
        Gdiplus::PointF c2(p2.x + (2.0f / 3.0f) * (ctrl.x - p2.x), p2.y + (2.0f / 3.0f) * (ctrl.y - p2.y));
        p.AddBezier(Gdiplus::PointF(p0.x, p0.y), c1, c2, Gdiplus::PointF(p2.x, p2.y));
    }

    static Pt projectHead3D(float x, float y, float z, float yawRad, float pitchRad) {
        float cy = std::cos(yawRad), sy = std::sin(yawRad);
        float x1 = x * cy + z * sy;
        float z1 = -x * sy + z * cy;
        float cp = std::cos(pitchRad), sp = std::sin(pitchRad);
        float y2 = y * cp + z1 * sp;
        return { x1, y2 };
    }

    static Pt quadPoint(Pt p0, Pt p1, Pt p2, float t) {
        float mt = 1.0f - t;
        return { mt * mt * p0.x + 2.0f * mt * t * p1.x + t * t * p2.x,
                 mt * mt * p0.y + 2.0f * mt * t * p1.y + t * t * p2.y };
    }

    static Pt quadTangent(Pt p0, Pt p1, Pt p2, float t) {
        float mt = 1.0f - t;
        return { 2.0f * mt * (p1.x - p0.x) + 2.0f * t * (p2.x - p1.x),
                 2.0f * mt * (p1.y - p0.y) + 2.0f * t * (p2.y - p1.y) };
    }

    // MARK: - Mini White Mouse (180° facing the cat)

    static void drawMouse(Gdiplus::Graphics& g, const CatPhysics& physics, KeyboardTheme theme) {
        using namespace Gdiplus;
        Pt center = { CatPhysics::MouseBaseCenter.x + physics.mouseOffset.point().x,
                      CatPhysics::MouseBaseCenter.y + physics.mouseOffset.point().y };
        float pressShift = std::max(physics.mousePressLeft.value, physics.mousePressRight.value) * 1.6f;

        GraphicsState st = g.Save();
        g.TranslateTransform(center.x, center.y + pressShift);
        g.RotateTransform(0.22f * RadToDeg);

        SolidBrush shadowBrush(Color(60, 0, 0, 0));
        g.FillEllipse(&shadowBrush, -13.5f, -9.5f, 27.0f, 31.0f);

        Color bodyCol, borderCol, wheelCol;
        if (theme == KeyboardTheme::MidnightHacker) {
            bodyCol = Color(255, 51, 54, 61); borderCol = Color(255, 20, 20, 26); wheelCol = Color(255, 199, 242, 64);
        } else if (theme == KeyboardTheme::RetroCream) {
            bodyCol = Color(255, 227, 222, 209); borderCol = Color(255, 51, 48, 46); wheelCol = Color(255, 224, 107, 61);
        } else {
            bodyCol = Color(255, 244, 242, 236); borderCol = Color(255, 36, 36, 41); wheelCol = Color(255, 245, 176, 41);
        }

        SolidBrush bodyBrush(bodyCol);
        g.FillEllipse(&bodyBrush, -13.5f, -16.5f, 27.0f, 33.0f);

        if (physics.mousePressLeft.value > 0.05f || physics.mousePressRight.value > 0.05f) {
            GraphicsPath clipPath;
            clipPath.AddEllipse(-13.5f, -16.5f, 27.0f, 33.0f);
            GraphicsState cs = g.Save();
            g.SetClip(&clipPath);
            if (physics.mousePressLeft.value > 0.05f) {
                SolidBrush hb(Color((BYTE)(96 * physics.mousePressLeft.value), 201, 242, 64));
                g.FillRectangle(&hb, 0.0f, 0.0f, 14.0f, 17.0f);
            }
            if (physics.mousePressRight.value > 0.05f) {
                SolidBrush hb(Color((BYTE)(96 * physics.mousePressRight.value), 201, 242, 64));
                g.FillRectangle(&hb, -14.0f, 0.0f, 14.0f, 17.0f);
            }
            g.Restore(cs);
        }

        // Front button split seam pointing away from cat (bottom)
        Pen seamPen(Color(82, borderCol.GetR(), borderCol.GetG(), borderCol.GetB()), 1.05f);
        g.DrawLine(&seamPen, 0.0f, 16.0f, 0.0f, 1.5f);

        // Amber scroll wheel on front (bottom) half
        GraphicsPath wheelPath;
        addRoundedRect(wheelPath, -1.9f, 2.5f, 3.8f, 8.5f, 1.9f);
        SolidBrush wheelBrush(wheelCol);
        g.FillPath(&wheelBrush, &wheelPath);

        Pen borderPen(borderCol, 1.7f);
        g.DrawEllipse(&borderPen, -13.5f, -16.5f, 27.0f, 33.0f);
        g.Restore(st);
    }

    // MARK: - Mini 65% Mechanical Keyboard (180° facing the cat)

    static void drawKeyboard(Gdiplus::Graphics& g, const CatPhysics& physics, KeyboardTheme theme, EyeColorTheme eyeTheme) {
        using namespace Gdiplus;
        Pt tl = KeyboardLayout::screenQuadPoint(-0.025f, -0.08f);
        Pt tr = KeyboardLayout::screenQuadPoint(1.025f, -0.08f);
        Pt bl = KeyboardLayout::screenQuadPoint(-0.032f, 1.08f);
        Pt br = KeyboardLayout::screenQuadPoint(1.032f, 1.08f);
        constexpr float depth = 6.5f;

        // Shadow
        {
            GraphicsPath sp;
            addQuadPoly(sp, {tl.x, tl.y + depth + 2}, {tr.x, tr.y + depth + 2},
                            {br.x, br.y + depth + 3}, {bl.x, bl.y + depth + 3});
            SolidBrush sb(Color(56, 0, 0, 0));
            g.FillPath(&sb, &sp);
        }

        Color baseCol, deckCol, keyCol, borderCol, accentCol;
        if (theme == KeyboardTheme::MidnightHacker) {
            baseCol = Color(255, 15, 15, 20); deckCol = Color(255, 26, 28, 33);
            keyCol = Color(255, 54, 56, 64); borderCol = Color(242, 28, 28, 33); accentCol = Color(255, 196, 240, 64);
        } else if (theme == KeyboardTheme::RetroCream) {
            baseCol = Color(255, 64, 61, 56); deckCol = Color(255, 97, 92, 84);
            keyCol = Color(255, 230, 224, 209); borderCol = Color(216, 173, 166, 153); accentCol = Color(255, 224, 107, 61);
        } else {
            baseCol = Color(255, 26, 26, 31); deckCol = Color(255, 36, 36, 41);
            keyCol = Color(255, 247, 246, 240); borderCol = Color(190, 199, 196, 189); accentCol = Color(255, 247, 179, 41);
        }

        {
            GraphicsPath lp;
            addQuadPoly(lp, {tl.x, tl.y + depth}, {tr.x, tr.y + depth}, {br.x, br.y + depth}, {bl.x, bl.y + depth});
            SolidBrush bb(baseCol);
            Pen bp(baseCol, 5.0f); bp.SetLineJoin(LineJoinRound);
            g.FillPath(&bb, &lp);
            g.DrawPath(&bp, &lp);
        }
        {
            GraphicsPath tp;
            addQuadPoly(tp, tl, tr, br, bl);
            SolidBrush db(deckCol);
            Pen dp(deckCol, 4.0f); dp.SetLineJoin(LineJoinRound);
            g.FillPath(&db, &tp);
            g.DrawPath(&dp, &tp);
        }

        constexpr float padU = 0.0028f;
        constexpr float padV = 0.014f;
        Color glowCol = eyePrimary(eyeTheme, 215);
        Pen keyBorderPen(borderCol, 0.7f);

        for (const auto& cap : KeyboardLayout::keycaps()) {
            float glow = 0.0f;
            auto it = physics.keyGlow.find(cap.key);
            if (it != physics.keyGlow.end()) glow = it->second;

            // 180° rotated to face the cat
            float su0 = (1.0f - (cap.xUnits + cap.widthUnits)) + padU;
            float su1 = (1.0f - cap.xUnits) - padU;
            float sv0 = (1.0f - (cap.row + 1) / 5.0f) + padV;
            float sv1 = (1.0f - cap.row / 5.0f) - padV;

            float dy = glow * 1.9f;
            Pt kTL = KeyboardLayout::screenQuadPoint(su0, sv0); kTL.y += dy;
            Pt kTR = KeyboardLayout::screenQuadPoint(su1, sv0); kTR.y += dy;
            Pt kBR = KeyboardLayout::screenQuadPoint(su1, sv1); kBR.y += dy;
            Pt kBL = KeyboardLayout::screenQuadPoint(su0, sv1); kBL.y += dy;

            GraphicsPath kp;
            addQuadPoly(kp, kTL, kTR, kBR, kBL);
            Color fill = cap.isAmberAccent ? accentCol : keyCol;
            if (glow > 0.02f) fill = blendColor(fill, glowCol, glow * 0.55f);

            SolidBrush kb(fill);
            g.FillPath(&kb, &kp);
            g.DrawPath(&keyBorderPen, &kp);
        }

        // Radiant backlit glow halos over pressed keys
        for (const auto& kv : physics.keyGlow) {
            if (kv.second <= 0.05f) continue;
            Pt c = KeyboardLayout::canvasPointForKey(kv.first);
            float r = 17.0f + kv.second * 6.0f;
            GraphicsPath gp;
            gp.AddEllipse(c.x - r, c.y - r, r * 2.0f, r * 2.0f);
            PathGradientBrush pgb(&gp);
            pgb.SetCenterColor(Color((BYTE)(92 * kv.second), glowCol.GetR(), glowCol.GetG(), glowCol.GetB()));
            Color surr[1] = { Color(0, glowCol.GetR(), glowCol.GetG(), glowCol.GetB()) };
            INT cnt = 1;
            pgb.SetSurroundColors(surr, &cnt);
            g.FillPath(&pgb, &gp);
        }
    }

    // MARK: - Tail

    static void drawTail(Gdiplus::Graphics& g, float seatX, float seatY, const CatPhysics& physics, CatCoatTheme coat) {
        using namespace Gdiplus;
        float p = physics.tailPhase;
        float hunt = std::clamp(physics.huntIntensity.value, 0.0f, 1.0f);
        float wag1 = std::sin(p) * (8.0f + hunt * 5.5f);
        float wag2 = std::cos(p * 1.3f) * (6.0f + hunt * 4.5f);
        float huntDrop = hunt * 14.0f;

        PointF root(seatX + 34.0f, seatY - 6.0f);
        PointF c1(seatX + 64.0f, seatY + 2.0f + wag1 * 0.35f + huntDrop * 0.4f);
        PointF c2(seatX + 66.0f + wag2 * 0.4f, seatY - 26.0f + wag1 * 0.7f + huntDrop);
        PointF tip(seatX + 91.0f + wag2 * 0.5f, seatY - 30.0f + wag1 * 0.85f + huntDrop);

        Pen tailPen(earTailColor(coat), 11.0f);
        tailPen.SetStartCap(LineCapRound);
        tailPen.SetEndCap(LineCapRound);
        tailPen.SetLineJoin(LineJoinRound);
        g.DrawBezier(&tailPen, root, c1, c2, tip);
        if (patterned(coat)) {
            GraphicsState state = g.Save();
            GraphicsPath outline; outline.AddBezier(root,c1,c2,tip); outline.Widen(&tailPen);
            g.SetClip(&outline, CombineModeIntersect);
            Pen ring(rgb(coat == CatCoatTheme::Shorthair ? 0x55595A : 0xA57543,128),3.5f);
            for (float t : {0.50f,0.78f}) {
                float u=1-t;
                float x=u*u*u*root.X+3*u*u*t*c1.X+3*u*t*t*c2.X+t*t*t*tip.X;
                float y=u*u*u*root.Y+3*u*u*t*c1.Y+3*u*t*t*c2.Y+t*t*t*tip.Y;
                float dx=3*u*u*(c1.X-root.X)+6*u*t*(c2.X-c1.X)+3*t*t*(tip.X-c2.X);
                float dy=3*u*u*(c1.Y-root.Y)+6*u*t*(c2.Y-c1.Y)+3*t*t*(tip.Y-c2.Y);
                float len=std::max(0.001f,std::hypot(dx,dy)),nx=-dy/len*8,ny=dx/len*8;
                g.DrawLine(&ring,x+nx,y+ny,x-nx,y-ny);
            }
            g.Restore(state);
        }
    }

    // MARK: - 3D-Projected Ears

    static void drawEarsLayer(Gdiplus::Graphics& g, Pt center, const CatPhysics& physics, CatCoatTheme coat) {
        using namespace Gdiplus;
        GraphicsState st = g.Save();
        g.TranslateTransform(center.x, center.y);
        g.RotateTransform(physics.headTilt.value * RadToDeg);

        float yawRad = physics.headYaw.value * 0.50f;
        float pitchRad = physics.headPitch.value * 0.36f;
        float fold = physics.earFold.value;
        float twitch = physics.earTwitch.value * 0.045f;

        drawEar3D(g, true, yawRad, pitchRad, fold, twitch, coat);
        drawEar3D(g, false, yawRad, pitchRad, fold, -twitch, coat);
        g.Restore(st);
    }

    static void drawEar3D(Gdiplus::Graphics& g, bool isLeft, float yawRad, float pitchRad, float fold, float twitch, CatCoatTheme coat) {
        using namespace Gdiplus;
        float sign = isLeft ? -1.0f : 1.0f;
        float foldAngle = sign * (0.06f + fold * 0.34f) + twitch;
        float tipDx = std::sin(foldAngle) * 32.0f;
        float tipDy = -std::cos(foldAngle) * 32.0f;

        Pt innerBase = projectHead3D(sign * 9.0f, -16.0f, 0.0f, yawRad, pitchRad);
        Pt outerBase = projectHead3D(sign * 35.0f, -5.0f, -2.0f, yawRad, pitchRad);
        Pt tip3D     = projectHead3D(sign * 24.5f + tipDx, -20.0f + tipDy, -3.0f, yawRad, pitchRad);

        Pt tipL = { tip3D.x - sign * 3.8f, tip3D.y + 2.8f };
        Pt tipR = { tip3D.x + sign * 3.8f, tip3D.y + 3.2f };

        GraphicsPath ear;
        addQuadBezier(ear, innerBase, { (innerBase.x + tipL.x) * 0.5f - sign * 1.5f, (innerBase.y + tipL.y) * 0.5f - 2.0f }, tipL);
        addQuadBezier(ear, tipL, { tip3D.x, tip3D.y - 2.2f }, tipR);
        addQuadBezier(ear, tipR, { (outerBase.x + tipR.x) * 0.5f + sign * 2.5f, (outerBase.y + tipR.y) * 0.5f - 2.0f }, outerBase);
        ear.CloseFigure();

        SolidBrush b(earTailColor(coat));
        g.FillPath(&b, &ear);
    }

    // MARK: - Unified C1-Continuous Cat Silhouette (+ Siamese 2.5D Face Mask)

    static void drawUnifiedSilhouette(Gdiplus::Graphics& g, float seatX, float seatY, Pt headCenter, float breath, const CatPhysics& physics, CatCoatTheme coat) {
        using namespace Gdiplus;
        float yaw = physics.headYaw.value;
        float pitch = physics.headPitch.value;
        float hTilt = physics.headTilt.value;
        float bTilt = physics.bodyTilt.value;

        float cH = std::cos(hTilt), sH = std::sin(hTilt);
        auto H = [&](float lx, float ly) -> PointF {
            return PointF(headCenter.x + lx * cH - ly * sH, headCenter.y + lx * sH + ly * cH);
        };

        float cB = std::cos(bTilt), sB = std::sin(bTilt);
        float sy = physics.bodySquash.value;
        float sx = 1.0f + (1.0f - sy) * 0.55f;
        auto B = [&](float lx, float ly) -> PointF {
            float rx = lx * sx, ry = ly * sy;
            return PointF(seatX + rx * cB - ry * sB, seatY + rx * sB + ry * cB);
        };

        float cheekShift = yaw * 3.0f;
        float crownDrop = pitch * 2.0f;
        float leftCheekX = -42.5f + cheekShift * 0.4f;
        float rightCheekX = 42.5f + cheekShift * 0.4f;

        PointF crown  = H(cheekShift * 0.2f, -30.0f + crownDrop);
        PointF rCheek = H(rightCheekX, 5.0f);
        PointF rHip   = B(49.0f, -14.0f);
        PointF rBot   = B(24.0f, 12.0f);
        PointF lBot   = B(-24.0f, 12.0f);
        PointF lHip   = B(-49.0f, -14.0f);
        PointF lCheek = H(leftCheekX, 5.0f);

        GraphicsPath path;
        path.AddBezier(crown, H(26.0f + cheekShift * 0.3f, -30.0f + crownDrop), H(rightCheekX + 1.5f, -14.0f), rCheek);
        path.AddBezier(rCheek, H(rightCheekX - 2.5f, 24.0f), B(51.5f, -48.0f + breath * 0.5f), rHip);
        path.AddBezier(rHip, B(47.0f, 5.0f), B(36.0f, 12.0f), rBot);
        path.AddLine(rBot, lBot);
        path.AddBezier(lBot, B(-36.0f, 12.0f), B(-47.0f, 5.0f), lHip);
        path.AddBezier(lHip, B(-51.5f, -48.0f + breath * 0.5f), H(leftCheekX + 2.5f, 24.0f), lCheek);
        path.AddBezier(lCheek, H(leftCheekX - 1.5f, -14.0f), H(-26.0f + cheekShift * 0.3f, -30.0f + crownDrop), crown);
        path.CloseFigure();

        if (coat != CatCoatTheme::Siamese) {
            SolidBrush b(bodyColor(coat));
            g.FillPath(&b, &path);
            if (patterned(coat)) {
                GraphicsState cs = g.Save(); g.SetClip(&path, CombineModeIntersect);
                drawCoatMarks(g, seatX, seatY, headCenter, physics, coat); g.Restore(cs);
            }
        } else {
            GraphicsState cs = g.Save();
            g.SetClip(&path);

            LinearGradientBrush bodyGrad(
                PointF(seatX, headCenter.y - 54.0f),
                PointF(seatX, seatY + 38.0f),
                siameseCream(),
                siameseLatte()
            );
            bodyGrad.SetWrapMode(WrapModeTileFlipXY);
            Color bodyCols[4] = { siameseCream(), siameseCream(), siameseLatte(), siameseLatte() };
            REAL bodyPos[4] = { 0.0f, 0.22f, 0.80f, 1.0f };
            bodyGrad.SetInterpolationColors(bodyCols, bodyPos, 4);
            g.FillPath(&bodyGrad, &path);

            // 2.5D Rotating Seal-Point "挖煤" Face Mask!
            float yawRad = yaw * 0.50f;
            float pitchRad = pitch * 0.36f;
            Pt maskC = projectHead3D(0.0f, 2.5f, 27.0f, yawRad, pitchRad);
            float maskSX = std::max(0.76f, std::cos(yawRad * 0.7f)) * 1.18f;
            float maskSY = std::max(0.82f, std::cos(pitchRad * 0.7f)) * 0.88f;

            GraphicsState ms = g.Save();
            g.TranslateTransform(headCenter.x, headCenter.y);
            g.RotateTransform(hTilt * RadToDeg);
            g.TranslateTransform(maskC.x, maskC.y);
            g.ScaleTransform(maskSX, maskSY);

            GraphicsPath maskEllipse;
            maskEllipse.AddEllipse(-31.0f, -31.0f, 62.0f, 62.0f);
            PathGradientBrush maskBrush(&maskEllipse);
            Color cols[4] = { Color(0, 115, 87, 69), Color(178, 115, 87, 69), siamesePoint(245), siamesePoint(255) };
            REAL pos[4] = { 0.0f, 0.26f, 0.56f, 1.0f };
            maskBrush.SetInterpolationColors(cols, pos, 4);
            g.FillPath(&maskBrush, &maskEllipse);

            g.Restore(ms);
            g.Restore(cs);

            // Clean subtle warm-cocoa outline so Siamese pops on white & dark wallpapers
            Pen outPen(siameseOutline(), 1.5f);
            g.DrawPath(&outPen, &path);
        }

        // Tucked hind paws at bottom
        GraphicsState ps = g.Save();
        g.TranslateTransform(seatX, seatY);
        g.RotateTransform(bTilt * RadToDeg);
        g.ScaleTransform(sx, sy);

        SolidBrush pawBrush(pawColor(coat));
        g.FillEllipse(&pawBrush, -35.0f, 3.0f, 17.0f, 11.0f);
        if (physics.hindFootActive.value < 0.35f) {
            g.FillEllipse(&pawBrush, 18.0f, 3.0f, 17.0f, 11.0f);
        }

        Pen rimPen(coat == CatCoatTheme::Siamese ? siameseCream(140) : creaseColor(coat,148), 1.3f);
        rimPen.SetStartCap(LineCapRound); rimPen.SetEndCap(LineCapRound);
        g.DrawArc(&rimPen, -34.0f, 1.0f, 15.0f, 15.0f, 15.0f, 150.0f);
        if (physics.hindFootActive.value < 0.35f) {
            g.DrawArc(&rimPen, 19.0f, 1.0f, 15.0f, 15.0f, 15.0f, 150.0f);
        }
        g.Restore(ps);
    }

    // MARK: - 3rd Key Hind Foot

    static void drawHindFoot(Gdiplus::Graphics& g, float seatX, float seatY, const CatPhysics& physics, CatCoatTheme coat) {
        Pt hip = { seatX + 26.0f, seatY + 2.0f };
        Pt target = physics.hindFoot.point();
        Pt mid = { (hip.x + target.x) * 0.5f + 10.0f, (hip.y + target.y) * 0.5f - 8.0f };
        drawLimb(g, hip, mid, target, 15.0f, 12.0f, true, 0.85f * physics.hindFootActive.value, coat);
    }

    // MARK: - 2.5D Eyes & Whiskers

    static void drawHeadFeatures(Gdiplus::Graphics& g, Pt center, const CatPhysics& physics, EyeColorTheme eyeTheme, CatCoatTheme coat) {
        using namespace Gdiplus;
        GraphicsState st = g.Save();
        g.TranslateTransform(center.x, center.y);
        g.RotateTransform(physics.headTilt.value * RadToDeg);

        float yawRad = physics.headYaw.value * 0.50f;
        float pitchRad = physics.headPitch.value * 0.36f;

        drawEye3D(g, true, yawRad, pitchRad, physics, eyeTheme, coat);
        drawEye3D(g, false, yawRad, pitchRad, physics, eyeTheme, coat);
        drawWhiskers3D(g, yawRad, pitchRad, physics, coat);
        g.Restore(st);
    }

    static void drawEye3D(Gdiplus::Graphics& g, bool isLeft, float yawRad, float pitchRad, const CatPhysics& physics, EyeColorTheme eyeTheme, CatCoatTheme coat) {
        using namespace Gdiplus;
        float sign = isLeft ? -1.0f : 1.0f;
        Color eyeCol = eyePrimary(eyeTheme);
        Pt proj = projectHead3D(sign * 17.2f, -2.0f, 27.5f, yawRad, pitchRad);

        float eyeNormal = yawRad + sign * 0.34f;
        float foreX = std::clamp(std::cos(eyeNormal) / std::cos(0.34f), 0.64f, 1.06f);
        float foreY = std::clamp(std::cos(pitchRad * 0.85f), 0.78f, 1.03f);

        GraphicsState st = g.Save();
        g.TranslateTransform(proj.x, proj.y);
        g.ScaleTransform(foreX, foreY);

        // Mode 1: Happy closed glowing curved line eyes (petting head!)
        if (physics.isHappyClosedEyes()) {
            GraphicsPath hp;
            addQuadBezier(hp, {-11.5f, 1.0f}, {sign * 1.5f, 3.4f}, {11.5f, 1.0f});
            Pen p(eyeCol, 2.6f);
            p.SetStartCap(LineCapRound); p.SetEndCap(LineCapRound);
            g.DrawPath(&p, &hp);
            g.Restore(st);
            return;
        }

        // Mode 2: Blissful half-squint glowing slits (petting body!)
        if (physics.isBlissfulSquint()) {
            GraphicsPath sp;
            addQuadBezier(sp, {-11.5f, -0.5f}, {0.0f, 0.6f}, {11.5f, -0.5f});
            addQuadBezier(sp, {11.5f, -0.5f}, {0.0f, 4.8f}, {-11.5f, -0.5f});
            sp.CloseFigure();
            SolidBrush sb(eyeCol);
            g.FillPath(&sb, &sp);
            g.Restore(st);
            return;
        }

        float hunt = std::clamp(physics.huntIntensity.value, 0.0f, 1.0f);
        g.RotateTransform(sign * (0.06f * (1.0f - hunt * 0.6f)) * RadToDeg);

        float rx = 12.0f + hunt * 1.1f;
        float ry = 10.8f + hunt * 1.8f;

        GraphicsPath eyeClip;
        eyeClip.AddEllipse(-rx, -ry, rx * 2.0f, ry * 2.0f);
        GraphicsState cs = g.Save();
        g.SetClip(&eyeClip, CombineModeIntersect);
        // Leave covered pixels untouched so the underlying face remains seamless.
        float closure = std::clamp(physics.effectiveEyeClosure(), 0.0f, 1.0f);
        if (closure >= 1.0f) {
            g.Restore(cs);
            g.Restore(st);
            return;
        }
        RectF opening(-rx - 2.0f, -ry + 2.0f * ry * closure,
                      2.0f * rx + 4.0f, 2.0f * ry * (1.0f - closure));
        if (closure > 0.0f) g.SetClip(opening, CombineModeIntersect);

        SolidBrush irisBrush(eyeCol);
        g.FillRectangle(&irisBrush, -rx - 1.0f, -ry - 1.0f, rx * 2.0f + 2.0f, ry * 2.0f + 2.0f);

        // Pupil morphs smoothly from vertical slit -> HUGE round black boba pupil!
        Pt gaze = physics.eyeGaze.point();
        float px = gaze.x * (4.2f - hunt * 1.2f);
        float py = gaze.y * (3.6f - hunt * 1.0f);
        float pScale = physics.pupilScale.value;
        float normalW = 5.0f * pScale;
        float normalH = 13.5f * std::min(1.12f, pScale);
        float huntDiam = 18.8f;
        float pupilW = normalW + (huntDiam - normalW) * hunt;
        float pupilH = normalH + (huntDiam - normalH) * hunt;

        // Reference iris colors, blended only in a narrow ring around the moving pupil.
        if (eyeTheme == EyeColorTheme::ReferenceHazel) {
            for (int i = 0; i <= 24; ++i) {
                float t = i / 24.0f, mix = t*t*(3-2*t), rim = (1-t)*2.2f;
                SolidBrush ring(Color(255, (BYTE)std::lround(218-77*mix),
                    (BYTE)std::lround(178-33*mix), (BYTE)std::lround(87+41*mix)));
                g.FillEllipse(&ring, px-pupilW*0.5f-rim, py-pupilH*0.5f-rim,
                    pupilW+2*rim, pupilH+2*rim);
            }
        }
        SolidBrush pupilBrush(catBlack());
        g.FillEllipse(&pupilBrush, px - pupilW * 0.5f, py - pupilH * 0.5f, pupilW, pupilH);

        // Tiny crisp white sparkle glint as boba hunting pupils dilate!
        if (hunt > 0.25f) {
            BYTE ga = (BYTE)(std::min(1.0f, (hunt - 0.25f) / 0.55f) * 235.0f);
            float gr = 2.1f + hunt * 0.9f;
            SolidBrush gb(Color(ga, 255, 255, 255));
            g.FillEllipse(&gb, px - pupilW * 0.22f - gr * 0.5f, py - pupilH * 0.22f - gr * 0.5f, gr * 2.0f, gr * 2.0f);
        }

        g.Restore(cs);
        g.Restore(st);
    }

    static void drawWhiskers3D(Gdiplus::Graphics& g, float yawRad, float pitchRad, const CatPhysics& physics, CatCoatTheme coat) {
        using namespace Gdiplus;
        float wiggle = std::sin(physics.time * 4.5f) * (std::max(physics.petHeadIntensity, physics.petBodyIntensity) * 1.2f);
        Pen pen(coat == CatCoatTheme::Siamese ? siameseWhisker() : (patterned(coat) ? rgb(0x8D8170,174) : whiskerWhite()), 1.05f);
        pen.SetStartCap(LineCapRound); pen.SetEndCap(LineCapRound);

        const float rows[3][3] = {
            { 10.5f, 36.5f, 8.8f + wiggle },
            { 14.2f, 37.8f, 14.2f + wiggle * 0.5f },
            { 18.0f, 35.5f, 19.6f - wiggle * 0.4f }
        };

        for (float sign : {-1.0f, 1.0f}) {
            for (int i = 0; i < 3; ++i) {
                Pt p0 = projectHead3D(sign * 24.0f, rows[i][0], 23.0f, yawRad, pitchRad);
                Pt p2 = projectHead3D(sign * rows[i][1], rows[i][2], 14.0f, yawRad, pitchRad);
                Pt ctrl = { (p0.x + p2.x) * 0.5f, (p0.y + p2.y) * 0.5f - 0.8f };
                GraphicsPath wp;
                addQuadBezier(wp, p0, ctrl, p2);
                g.DrawPath(&pen, &wp);
            }
        }
    }

    // MARK: - Front Paws

    static void drawFrontPaws(Gdiplus::Graphics& g, float seatX, float seatY, float breath, const CatPhysics& physics, CatCoatTheme coat) {
        Pt lPos = physics.leftPaw.point();
        Pt rPos = physics.rightPaw.point();

        float lDist = std::hypot(lPos.x - CatPhysics::LeftPawRest.x, lPos.y - CatPhysics::LeftPawRest.y);
        float rDist = std::hypot(rPos.x - CatPhysics::RightPawRest.x, rPos.y - CatPhysics::RightPawRest.y);

        auto shoulder = [&](float x) -> Pt {
            float sy=physics.bodySquash.value, sx=1+(1-sy)*0.55f;
            float c=std::cos(physics.bodyTilt.value), s=std::sin(physics.bodyTilt.value);
            float floor=126+physics.headOffsetY.value+physics.headPitch.value*4.8f+breath*1.1f+52;
            return {seatX+x*sx*c+43*sy*s, std::max(seatY+x*sx*s-43*sy*c,floor)};
        };
        Pt leftShoulder=shoulder(-20), rightShoulder=shoulder(20);

        if (lDist < 6.0f && !physics.leftPawOnMouse && !physics.leftPawTyping) {
            drawIdleFrontLeg(g, { seatX - 18.0f, seatY - 30.0f }, { seatX - 16.0f, seatY + 12.0f }, true, coat);
        } else {
            Pt mid = { (leftShoulder.x + lPos.x) * 0.5f - (physics.leftPawOnMouse ? 8.0f : 4.0f),
                       (leftShoulder.y + lPos.y) * 0.5f - 6.0f };
            drawLimb(g, leftShoulder, mid, lPos, 19.0f, 13.5f, false, 0.78f, coat);
        }

        if (rDist < 6.0f && !physics.rightPawTyping) {
            drawIdleFrontLeg(g, { seatX + 18.0f, seatY - 30.0f }, { seatX + 16.0f, seatY + 12.0f }, false, coat);
        } else {
            Pt mid = { (rightShoulder.x + rPos.x) * 0.5f + 4.0f, (rightShoulder.y + rPos.y) * 0.5f - 6.0f };
            drawLimb(g, rightShoulder, mid, rPos, 19.0f, 13.5f, true, 0.78f, coat);
        }
    }

    static void drawIdleFrontLeg(Gdiplus::Graphics& g, Pt top, Pt bottom, bool isLeft, CatCoatTheme coat) {
        using namespace Gdiplus;
        float sign = isLeft ? -1.0f : 1.0f;

        if (coat == CatCoatTheme::Siamese) {
            GraphicsPath lp;
            addRoundedRect(lp, bottom.x - 7.5f, top.y, 15.0f, bottom.y - top.y + 1.0f, 6.5f);
            float pad = 10.0f;
            float span = std::max(1.0f, (bottom.y + 2.0f) - top.y);
            float total = span + pad * 2.0f;
            LinearGradientBrush grad(PointF(bottom.x, top.y - pad), PointF(bottom.x, bottom.y + 2.0f + pad), siameseCream(0), siamesePoint(255));
            grad.SetWrapMode(WrapModeTileFlipXY);
            Color cols[5] = { siameseCream(0), siameseCream(0), siameseMocha(190), siamesePoint(255), siamesePoint(255) };
            REAL pos[5] = { 0.0f, pad / total, (pad + span * 0.52f) / total, (pad + span * 0.85f) / total, 1.0f };
            grad.SetInterpolationColors(cols, pos, 5);
            g.FillPath(&grad, &lp);
        }

        if (patterned(coat)) {
            SolidBrush cuff(pawColor(coat));
            g.FillRectangle(&cuff, bottom.x-7.5f, bottom.y-13.0f, 15.0f, 13.0f);
            g.FillEllipse(&cuff, bottom.x-7.5f, bottom.y-5.0f, 15.0f, 10.0f);
        }
        Pen p(coat == CatCoatTheme::Siamese ? siameseOutline(158) : creaseColor(coat), 1.45f);
        p.SetStartCap(LineCapRound); p.SetEndCap(LineCapRound);

        GraphicsPath p1;
        addQuadBezier(p1, { top.x + sign * 8.5f, top.y }, { top.x + sign * 9.5f, (top.y + bottom.y) * 0.5f }, { bottom.x + sign * 7.5f, bottom.y - 1.0f });
        g.DrawPath(&p, &p1);

        GraphicsPath p2;
        addQuadBezier(p2, { top.x - sign * 7.0f, top.y + 4.0f }, { top.x - sign * 6.5f, (top.y + bottom.y) * 0.5f }, { bottom.x - sign * 6.0f, bottom.y - 1.0f });
        g.DrawPath(&p, &p2);
    }

    static void drawLimb(Gdiplus::Graphics& g, Pt start, Pt ctrl, Pt end, float startW, float endW, bool outlineOnLeft, float outlineAlpha, CatCoatTheme coat) {
        using namespace Gdiplus;
        constexpr int steps = 14;
        std::vector<PointF> leftEdge, rightEdge;
        leftEdge.reserve(steps + 1);
        rightEdge.reserve(steps + 1);

        for (int i = 0; i <= steps; ++i) {
            float t = (float)i / (float)steps;
            Pt p = quadPoint(start, ctrl, end, t);
            Pt tan = quadTangent(start, ctrl, end, t);
            float len = std::max(0.001f, std::hypot(tan.x, tan.y));
            float nx = -tan.y / len, ny = tan.x / len;
            float hw = (startW + (endW - startW) * t) * 0.5f;
            leftEdge.push_back(PointF(p.x + nx * hw, p.y + ny * hw));
            rightEdge.push_back(PointF(p.x - nx * hw, p.y - ny * hw));
        }

        std::vector<PointF> poly = leftEdge;
        for (int i = steps; i >= 0; --i) poly.push_back(rightEdge[i]);

        GraphicsPath limbPath;
        limbPath.AddLines(poly.data(), (INT)poly.size());
        Pt tangent=quadTangent(start,ctrl,end,0);
        float length=std::max(0.001f,std::hypot(tangent.x,tangent.y));
        addQuadBezier(limbPath, {rightEdge[0].X,rightEdge[0].Y},
            {start.x-tangent.x/length*startW*0.5f,start.y-tangent.y/length*startW*0.5f},
            {leftEdge[0].X,leftEdge[0].Y});
        limbPath.CloseFigure();

        float pawR = endW * 0.58f;
        RectF pawRect(end.x - pawR, end.y - pawR * 0.85f, pawR * 2.0f, pawR * 1.7f);

        if (coat != CatCoatTheme::Siamese) {
            SolidBrush b(limbColor(coat));
            if (patterned(coat)) {
                LinearGradientBrush gradient(PointF(start.x,start.y),PointF(end.x,end.y),pawColor(coat),limbColor(coat));
                gradient.SetWrapMode(WrapModeTileFlipXY);
                Color colors[3]={pawColor(coat),limbColor(coat),limbColor(coat)};
                REAL positions[3]={0,0.30f,1};gradient.SetInterpolationColors(colors,positions,3);
                g.FillPath(&gradient,&limbPath);
            } else g.FillPath(&b, &limbPath);
            if (!patterned(coat) && coat != CatCoatTheme::SlenderTuxedo) g.FillEllipse(&b, pawRect);
            else {
                std::vector<PointF> points; int first = (int)(steps*0.72f);
                for (int i=first; i<=steps; ++i) points.push_back(leftEdge[i]);
                for (int i=steps; i>=first; --i) points.push_back(rightEdge[i]);
                SolidBrush cuff(pawColor(coat));
                g.FillPolygon(&cuff, points.data(), (INT)points.size()); g.FillEllipse(&cuff, pawRect);
            }
        } else {
            // Pad gradient endpoints beyond limb geometry + use WrapModeTileFlipXY so GDI+
            // NEVER wraps t < 0 shoulder corners around to t = 1.0 dark espresso!
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            float dist = std::hypot(dx, dy);
            float ux = (dist > 1.0f) ? (dx / dist) : 0.0f;
            float uy = (dist > 1.0f) ? (dy / dist) : 1.0f;
            dist = std::max(1.0f, dist);
            float padStart = startW + 18.0f;
            float padEnd = endW + 18.0f;
            float totalLen = padStart + dist + padEnd;

            PointF gStart(start.x - ux * padStart, start.y - uy * padStart);
            PointF gEnd(end.x + ux * padEnd, end.y + uy * padEnd);

            LinearGradientBrush grad(gStart, gEnd, siameseCream(0), siamesePoint(255));
            grad.SetWrapMode(WrapModeTileFlipXY);
            Color cols[6] = {
                siameseCream(0),
                siameseCream(0),
                siameseCream(255),
                siameseMocha(255),
                siamesePoint(255),
                siamesePoint(255)
            };
            REAL pos[6] = {
                0.0f,
                padStart / totalLen,
                (padStart + dist * 0.16f) / totalLen,
                (padStart + dist * 0.48f) / totalLen,
                (padStart + dist * 0.78f) / totalLen,
                1.0f
            };
            grad.SetInterpolationColors(cols, pos, 6);
            g.FillPath(&grad, &limbPath);

            SolidBrush pawBrush(siamesePoint());
            g.FillEllipse(&pawBrush, pawRect);

            const auto& outerEdge = outlineOnLeft ? rightEdge : leftEdge;
            Pen op(siameseOutline(184), 1.25f);
            op.SetStartCap(LineCapRound); op.SetEndCap(LineCapRound); op.SetLineJoin(LineJoinRound);
            g.DrawLines(&op, outerEdge.data() + 3, (INT)outerEdge.size() - 4);
        }

        const auto& hiEdge = outlineOnLeft ? leftEdge : rightEdge;
        int startIdx = 2;
        int endIdx = (int)((hiEdge.size() - 1) * 0.68f);
        if (endIdx > startIdx) {
            Color sc = (coat == CatCoatTheme::Siamese)
                ? siameseOutline((BYTE)(outlineAlpha * 199))
                : creaseColor(coat, (BYTE)(outlineAlpha * 229));
            Pen hp(sc, 1.35f);
            hp.SetStartCap(LineCapRound); hp.SetEndCap(LineCapRound); hp.SetLineJoin(LineJoinRound);
            g.DrawLines(&hp, hiEdge.data() + startIdx, endIdx - startIdx + 1);
        }
    }

    // Photo-inspired skin, isolated from the four existing coat render paths.
    // Uses the same spring state and paw targets as the original animation.
    static void drawSlenderTuxedo(Gdiplus::Graphics& g, float seatX, float seatY,
        Pt head, const CatPhysics& p, const CatSettings& settings) {
        using namespace Gdiplus;
        constexpr CatCoatTheme coat = CatCoatTheme::SlenderTuxedo;
        const float sy=p.bodySquash.value, sx=1+(1-sy)*0.55f;
        const float bc=std::cos(p.bodyTilt.value), bs=std::sin(p.bodyTilt.value);
        const float hc=std::cos(p.headTilt.value), hs=std::sin(p.headTilt.value);
        auto B = [&](float x, float y) -> PointF {
            return {seatX+x*sx*bc-y*sy*bs, seatY+x*sx*bs+y*sy*bc};
        };
        auto H = [&](float x, float y) -> PointF {
            return {head.x+x*hc-y*hs, head.y+x*hs+y*hc};
        };
        SolidBrush black(catBlack()), white(rgb(0xF2EEE4)), pink(rgb(0xDF999C));
        drawTail(g,seatX,seatY,p,coat);

        // Large ears retain the original yaw, pitch, fold and twitch motion.
        GraphicsState ears=g.Save();
        g.TranslateTransform(head.x,head.y);
        g.RotateTransform(p.headTilt.value*RadToDeg);
        g.ScaleTransform(1.32f,1.18f);
        drawEar3D(g,true,p.headYaw.value*0.50f,p.headPitch.value*0.36f,
            p.earFold.value,p.earTwitch.value*0.045f,coat);
        drawEar3D(g,false,p.headYaw.value*0.50f,p.headPitch.value*0.36f,
            p.earFold.value,-p.earTwitch.value*0.045f,coat);
        g.Restore(ears);

        GraphicsPath silhouette;
        silhouette.AddBezier(H(0,-29),H(21,-29),H(38,-17),H(38,2));
        silhouette.AddBezier(H(38,2),H(38,19),H(27,30),H(16,32));
        silhouette.AddBezier(H(16,32),B(20,-65),B(32,-39),B(31,-8));
        silhouette.AddBezier(B(31,-8),B(30,10),B(21,16),B(14,16));
        silhouette.AddLine(B(14,16),B(-14,16));
        silhouette.AddBezier(B(-14,16),B(-21,16),B(-30,10),B(-31,-8));
        silhouette.AddBezier(B(-31,-8),B(-32,-39),B(-20,-65),H(-16,32));
        silhouette.AddBezier(H(-16,32),H(-27,30),H(-38,19),H(-38,2));
        silhouette.AddBezier(H(-38,2),H(-38,-17),H(-21,-29),H(0,-29));
        silhouette.CloseFigure();
        g.FillPath(&black,&silhouette);
        GraphicsState clipped=g.Save();
        g.SetClip(&silhouette,CombineModeIntersect);

        // Bib follows the body; upper edge follows the moving jaw.
        GraphicsPath bib;
        bib.AddBezier(H(-15,25),H(-5,31),H(5,31),H(15,25));
        bib.AddBezier(H(15,25),B(20,-51),B(13,-15),B(0,4));
        bib.AddBezier(B(0,4),B(-13,-7),B(-20,-51),H(-15,25));
        bib.CloseFigure();g.FillPath(&white,&bib);

        // Markings and facial features share the same projected surface.
        g.TranslateTransform(head.x,head.y);
        g.RotateTransform(p.headTilt.value*RadToDeg);
        g.ScaleTransform(0.78f,0.82f);
        auto F = [&](float x,float y) -> PointF {
            Pt q=projectHead3D(x,y,27.5f,p.headYaw.value*0.50f,p.headPitch.value*0.36f);
            return {q.x,q.y};
        };
        GraphicsPath blaze;
        blaze.AddBezier(F(2,-26),F(12,-24),F(8,-9),F(9,6));
        blaze.AddBezier(F(9,6),F(22,14),F(34,14),F(45,17));
        blaze.AddLine(F(45,17),F(50,48));
        blaze.AddLine(F(50,48),F(-50,48));
        blaze.AddLine(F(-50,48),F(-45,17));
        blaze.AddBezier(F(-45,17),F(-29,14),F(-20,12),F(-9,7));
        blaze.AddBezier(F(-9,7),F(-4,-4),F(-5,-16),F(2,-26));
        blaze.CloseFigure();g.FillPath(&white,&blaze);
        GraphicsPath chin;
        chin.AddBezier(F(-6,29),F(-2,27),F(2,27),F(6,29));
        chin.AddBezier(F(6,29),F(12,34),F(-12,34),F(-6,29));
        chin.CloseFigure();g.FillPath(&black,&chin);
        drawEye3D(g,true,p.headYaw.value*0.50f,p.headPitch.value*0.36f,p,settings.eyeTheme,coat);
        drawEye3D(g,false,p.headYaw.value*0.50f,p.headPitch.value*0.36f,p,settings.eyeTheme,coat);
        GraphicsPath nose;
        PointF nosePoints[]={F(-4,17),F(4,17),F(0,21)};
        nose.AddPolygon(nosePoints,3);g.FillPath(&pink,&nose);
        Pen mouth(rgb(0x514443),0.8f);
        g.DrawLine(&mouth,F(0,21),F(0,24));
        GraphicsPath smile;
        smile.AddBezier(F(-5,24),F(-2,27),F(2,27),F(5,24));
        g.DrawPath(&mouth,&smile);
        g.Restore(clipped);

        // Whiskers extend outside the silhouette instead of being clipped.
        GraphicsState whiskers=g.Save();
        g.TranslateTransform(head.x,head.y);g.RotateTransform(p.headTilt.value*RadToDeg);
        g.ScaleTransform(0.78f,0.82f);
        drawWhiskers3D(g,p.headYaw.value*0.50f,p.headPitch.value*0.36f,p,coat);
        g.Restore(whiskers);
        for (float x : {-18.0f,18.0f}) {
            if (x>0 && p.hindFootActive.value>=0.35f) continue;
            PointF q=B(x,12);g.FillEllipse(&white,q.X-7.0f,q.Y-4.5f,14.0f,9.0f);
        }
        if (p.hindFootActive.value>0.03f) {
            PointF q=B(18,2);Pt target=p.hindFoot.point();
            drawLimb(g,{q.X,q.Y},{(q.X+target.x)*0.5f+8,(q.Y+target.y)*0.5f-8},
                target,11,10,true,0.85f*p.hindFootActive.value,coat);
        }
        for (int side=0;side<2;++side) {
            float sign=side==0 ? -1.0f : 1.0f;
            bool idle=side==0 ? (!p.leftPawOnMouse && !p.leftPawTyping) : !p.rightPawTyping;
            PointF shoulder=B(sign*14,-51), rest=B(sign*16,12);
            Pt target=side==0 ? p.leftPaw.point() : p.rightPaw.point();
            Pt oldRest=side==0 ? CatPhysics::LeftPawRest : CatPhysics::RightPawRest;
            if (idle && std::hypot(target.x-oldRest.x,target.y-oldRest.y)<6)
                target={rest.X,rest.Y};
            Pt mid={(shoulder.X+target.x)*0.5f+sign*3,(shoulder.Y+target.y)*0.5f-4};
            drawLimb(g,{shoulder.X,shoulder.Y},mid,target,12,12,side!=0,0.60f,coat);
        }
    }

    // MARK: - Floating Mechanical Keycaps

    static void drawFloatingKeycaps(Gdiplus::Graphics& g, const CatPhysics& physics) {
        using namespace Gdiplus;
        FontFamily ff(L"Segoe UI");
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        for (const auto& b : physics.floatingKeycaps) {
            float prog = b.age / b.lifetime;
            float scale = 1.0f;
            if (b.age < 0.09f) {
                scale = 0.72f + 0.38f * std::sin((b.age / 0.09f) * 3.14159f * 0.65f);
            } else if (b.age < 0.18f) {
                scale = 1.10f - 0.10f * ((b.age - 0.09f) / 0.09f);
            }

            float alphaF = prog < 0.62f ? 1.0f : std::max(0.0f, 1.0f - (prog - 0.62f) / 0.38f);
            BYTE a = (BYTE)(alphaF * 255.0f);
            if (a < 4) continue;

            size_t len = b.label.size();
            float w = len <= 1 ? 23.0f : (len == 2 ? 30.0f : 38.0f);
            float h = 22.0f;

            GraphicsState st = g.Save();
            g.TranslateTransform(b.position.x, b.position.y);
            g.RotateTransform(b.rotation * RadToDeg);
            g.ScaleTransform(scale, scale);

            GraphicsPath basePath;
            addRoundedRect(basePath, -w * 0.5f, -h * 0.5f + 2.5f, w, h, 5.5f);
            SolidBrush baseBrush(Color((BYTE)(alphaF * 242), 38, 38, 43));
            g.FillPath(&baseBrush, &basePath);

            GraphicsPath facePath;
            addRoundedRect(facePath, -w * 0.5f, -h * 0.5f, w, h, 5.2f);
            SolidBrush faceBrush(Color(a, 249, 248, 243));
            Pen facePen(Color(a, 41, 41, 46), 1.45f);
            g.FillPath(&faceBrush, &facePath);
            g.DrawPath(&facePen, &facePath);

            Font font(&ff, len <= 1 ? 11.0f : 9.2f, FontStyleBold, UnitPixel);
            SolidBrush txtBrush(Color(a, 31, 31, 36));
            g.DrawString(b.label.c_str(), -1, &font, RectF(-w * 0.5f, -h * 0.5f, w, h), &sf, &txtBrush);

            g.Restore(st);
        }
    }

    // MARK: - Petting Sparkles & Sleep Z

    static void drawSparkles(Gdiplus::Graphics& g, const CatPhysics& physics, EyeColorTheme eyeTheme) {
        using namespace Gdiplus;
        Color baseCol = eyePrimary(eyeTheme);
        FontFamily ff(L"Segoe UI");

        for (const auto& sp : physics.sparkles) {
            float p = std::min(1.0f, sp.age / sp.lifetime);
            BYTE a = (BYTE)(std::sin(p * 3.14159f) * 215.0f);
            if (a < 4) continue;

            Color c(a, baseCol.GetR(), baseCol.GetG(), baseCol.GetB());
            if (sp.isSleepZ) {
                Font f(&ff, 11.0f + p * 3.0f, FontStyleBold, UnitPixel);
                SolidBrush sb(c);
                g.DrawString(L"z", -1, &f, PointF(sp.position.x, sp.position.y), &sb);
            } else {
                float r = 3.5f + p * 1.5f;
                GraphicsState st = g.Save();
                g.TranslateTransform(sp.position.x, sp.position.y);
                GraphicsPath star;
                addQuadBezier(star, {0, -r}, {0.8f, -0.8f}, {r, 0});
                addQuadBezier(star, {r, 0}, {0.8f, 0.8f}, {0, r});
                addQuadBezier(star, {0, r}, {-0.8f, 0.8f}, {-r, 0});
                addQuadBezier(star, {-r, 0}, {-0.8f, -0.8f}, {0, -r});
                star.CloseFigure();
                SolidBrush sb(c);
                g.FillPath(&sb, &star);
                g.Restore(st);
            }
        }
    }
};
