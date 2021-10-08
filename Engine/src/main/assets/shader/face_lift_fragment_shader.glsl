precision mediump float;

varying vec2 vTextureCoordinate;
uniform sampler2D sTextureSampler;

uniform float uImgWidth;
uniform float uImgHeight;

uniform vec2 uLhsDstPt;
uniform vec2 uRhsDstPt;
uniform vec2 uLhsCtrlPt;
uniform vec2 uRhsCtrlPt;
uniform float uIntensity;

vec2 stretch(vec2 curPt, vec2 ctrlPt, vec2 dstPt, float aspectRatio, float radius, float intensity) {
    vec2 direction = dstPt - ctrlPt;
    float infect = distance(curPt, ctrlPt) / radius;
    infect = pow(1.0 - infect, 2.0);
    infect = infect * intensity * 0.2;
    infect = clamp(infect, 0.0, 1.0);
    vec2 offset = direction * infect;
    vec2 result = curPt - offset;
    return result;
}

void main() {
    float imgRatio = uImgHeight / uImgWidth;

    vec2 current_pt = vec2(vTextureCoordinate.x, vTextureCoordinate.y);
    current_pt.y = current_pt.y * imgRatio + 0.5 - 0.5 * imgRatio;

    vec2 trans_Lhs_Dst_Pt = vec2(uLhsDstPt.x / uImgWidth, uLhsDstPt.y / uImgHeight);
    trans_Lhs_Dst_Pt.y = trans_Lhs_Dst_Pt.y * imgRatio + 0.5 - 0.5 * imgRatio;

    vec2 trans_Rhs_Dst_Pt = vec2(uRhsDstPt.x / uImgWidth, uRhsDstPt.y / uImgHeight);
    trans_Rhs_Dst_Pt.y = trans_Rhs_Dst_Pt.y * imgRatio + 0.5 - 0.5 * imgRatio;

    vec2 trans_Lhs_Ctrl_Pt = vec2(uLhsCtrlPt.x / uImgWidth, uLhsCtrlPt.y / uImgHeight);
    trans_Lhs_Ctrl_Pt.y = trans_Lhs_Ctrl_Pt.y * imgRatio + 0.5 - 0.5 * imgRatio;

    vec2 trans_Rhs_Ctrl_Pt = vec2(uRhsCtrlPt.x / uImgWidth, uRhsCtrlPt.y / uImgHeight);
    trans_Rhs_Ctrl_Pt.y = trans_Rhs_Ctrl_Pt.y * imgRatio + 0.5 - 0.5 * imgRatio;

    float left_radius = distance(trans_Lhs_Dst_Pt, trans_Lhs_Ctrl_Pt);
    float right_radius = distance(trans_Rhs_Dst_Pt, trans_Rhs_Ctrl_Pt);

    float left_dist = distance(current_pt, trans_Lhs_Ctrl_Pt);
    float right_dist = distance(current_pt, trans_Rhs_Ctrl_Pt);

    if (left_dist < left_radius) {
        vec2 stretched_pt = stretch(current_pt, trans_Lhs_Ctrl_Pt, trans_Lhs_Dst_Pt, imgRatio, left_radius, uIntensity);
        stretched_pt.y = (stretched_pt.y + 0.5 * imgRatio - 0.5) / imgRatio;
        gl_FragColor = texture2D(sTextureSampler, stretched_pt);
    } else if (right_dist < right_radius) {
        vec2 stretched_pt = stretch(current_pt, trans_Rhs_Ctrl_Pt, trans_Rhs_Dst_Pt, imgRatio, right_radius, uIntensity);
        stretched_pt.y = (stretched_pt.y + 0.5 * imgRatio - 0.5) / imgRatio;
        gl_FragColor = texture2D(sTextureSampler, stretched_pt);
    } else {
        gl_FragColor = texture2D(sTextureSampler, vTextureCoordinate);
    }
}