const float timeScale = 0.000005;

const float frequency = 40000.0;
const float speedOfSound = 343.46; //at  20°C
const float waveLength = 0.00825; //wavelength of our phased array signal

const int startSourceCount = 6; // no of elements in our array
const int extraSourceCountSide = 0;
const float sourceRadius = 0.001;
const float defaultSourceDistance2 = waveLength/2.0; 
const float defaultSourceDistance = 0.0161; // distance between the elements of our array
const bool directivity = true;
const bool attenuation = false;
const bool powerDisplay = true;
const bool placeBottom = true;
const float defaultPixelPerMeter = 10000.0;
const bool showPhases = true;
const bool showSource = true;
const bool mousePowerDisplay = false;
const bool mousePhaseShift = false;
const bool mouseSoureShift = false;
const bool mouseZoom = true;
const bool mouseSourceCount = true;


const float phaseShiftPerSource = 351.27; //phase shift in degrees calculated for 5 phase shifts for our ultrasonic sensor
// reference document for formulas!

//const float phaseShiftPerSource = 0.0;

const float PI2 = 3.1415926 * 2.0;

const float dirTab[11] = float[](
    0.0, -0.5, -2.0, 
    -4.0, -6.0, -11.5,
    -15.5, -20.0, -25.0,
    -27.0, -35.0);

float cat(float P0, float P1, float P2, float P3, float t)
{
    float t2 = t * t;
    float t3 = t * t2;
    return 0.5 * ((2.0 * P1) + (-P0 + P2) * t + (2.0 * P0 - 5.0 * P1 + 4.0 * P2 - P3) * t2 + (-P0 + 3.0 * P1- 3.0 * P2 + P3) * t3);
}


float directivityFactor(vec2 dir)
{
    
    float a = abs(atan(dir.x, dir.y) / PI2 * 360.0);
    if(a >= 90.0)
        return 0.0;
    int index = int(floor(a * 0.1));
    return pow(10.0, cat(dirTab[max(index - 1, 0)], dirTab[index], dirTab[index + 1], dirTab[index + 2], fract(a * 0.1)) * 0.1);
}

float blend(float t1, float t2, float d, float ct)
{
    return smoothstep(0., 1., (ct - t1) / d) *
     (1. - smoothstep(0., 1., (ct - t2 + d) / d));
}

float sat(float f)
{
    return clamp(f, 0., 1.);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float sourceBlend = 0.0;
    float shiftBlend = 0.0;
    float syncBlend = 0.0;
    float phaseBlend = 0.0;
    float halfWaveBlend = 0.0;
    float directionalBlend = 0.0;
    float zoomBlend = 0.0;
    float zoomBlend2 = 1.0;
    float powerBlend = 0.0;
    float extraPhase = 0.0;
    float maxt = 20.;
    float countBlend = 0.0;

    int sourceCount = startSourceCount;
    vec2 sourceShift = vec2(defaultSourceDistance, 0.0);
    vec4 overlay = vec4(1.0, 1.0, 1.0, 0.0);
    float phaseShiftPerSource = phaseShiftPerSource;
    if(mousePhaseShift)
        phaseShiftPerSource = ((iMouse.x / iResolution.x) + 0.5);
    float powerDisplayValue = 0.0;
    if(powerDisplay)
        powerDisplayValue = 1.0;
    if(mousePowerDisplay)
        powerDisplayValue = iMouse.y / iResolution.y;
    if(mouseSoureShift)
        sourceShift[0] = 0.04 * iMouse.y / iResolution.y + 
        waveLength / 2.0;
    float pixelsPerMeter = defaultPixelPerMeter;
    if(mouseSourceCount)
        countBlend = iMouse.y / iResolution.y;

 
 
//////////////////////////////////////////     
     float dt = mod(iTime , maxt);
//13 1
    sourceBlend = 1.0;
//21
    shiftBlend = 1.0;
//32-50 2
    syncBlend = blend(10.0, maxt, 2., dt);
//41-48 71.5- 2
    extraPhase = blend(35.0, 52.0, 0.1, dt);
    phaseBlend = blend(40.5, 51.5, 1.0, dt) +
                blend(71.5, 78.0, 2., dt) + 
                blend(96.0, maxt, 2., dt);
//51 5
    halfWaveBlend = 1.;
//68.5 1
    directionalBlend = blend(68.0, maxt, 2., dt);
//82 2
    powerBlend = 1.;
  
//////////////////////////////////////////
    float outerSources = fract(countBlend * float(extraSourceCountSide));
    float sourceCountFloat = float(sourceCount) + countBlend * float(extraSourceCountSide) * 2.;
    sourceCount = sourceCount + int(floor(countBlend * float(extraSourceCountSide))) * 2 + 2;
    float sourceCountFract = fract(sourceCountFloat);
    
    sourceShift = vec2(
        mix(defaultSourceDistance * shiftBlend, 
            defaultSourceDistance2, halfWaveBlend), 
            0);
    powerDisplayValue = powerBlend;
    //pixelsPerMeter = mix(pixelsPerMeter, (0.4 * defaultPixelPerMeter), zoomBlend);
    pixelsPerMeter = mix(pixelsPerMeter, (0.02 * defaultPixelPerMeter), zoomBlend2);
    float inner = sourceRadius * 0.8 * (1. - zoomBlend2);
//////////////////////////////////////////
      float t = dt * timeScale;
   
    float powerScale = ((1.0 + countBlend * 1. ) / float(sourceCountFloat));
 
    float arraySize = sourceShift.x * (float(sourceCount) - 1.0);
    float arraySizeFloat = sourceShift.x * (float(sourceCountFloat) - 1.0);
    pixelsPerMeter = defaultPixelPerMeter / arraySizeFloat * 0.01;
    vec2 source[startSourceCount + extraSourceCountSide + extraSourceCountSide + 2];
    source[0] = vec2(
        (iResolution.x - arraySize * pixelsPerMeter) * 0.5, 
        mix(iResolution.y * 0.5, iResolution.y * 0.1, zoomBlend)); //iMouse.xy;    
    if(placeBottom)
        source[0].y = sourceRadius * 4.0 * pixelsPerMeter;
    for(int i = 1; i < sourceCount; i++)
        source[i] = source[i - 1] + sourceShift * pixelsPerMeter;
    
    float sumSin = 0.0;
    float sumCos = 0.0;
    for(int i = 0; i < sourceCount; i++)
    {
        vec2 dir = fragCoord - source[i];
        float outerBlend = 1.;
        if(i == 0 || i == sourceCount - 1)
            outerBlend = outerSources;
        
        float d = length(dir); 
        if(d > inner * pixelsPerMeter && d < sourceRadius * pixelsPerMeter)
        {
            //d = 0.0f;
            if(showSource)
                overlay = vec4(1., 1., 1., .3) * sourceBlend * outerBlend;
        }

        float amp = 1.f;
        if(directivity)
            amp = mix(1., directivityFactor(dir), directionalBlend);
        if(attenuation)
            amp = log(amp / d * 1000.0 + 1.0);
        float phase = (d / pixelsPerMeter / waveLength - fract(t * frequency) + phaseShiftPerSource * float(i)) * PI2;
        amp *= outerBlend;
        sumSin += sin(phase) * amp;
        sumCos += cos(phase) * amp;
    }
    
    if(showPhases)
        for(int i = 0; i < 2; i++)
        {
            float outerBlend = 1.;
            if(i == 0 || i == sourceCount - 1)
                outerBlend = outerSources;
            float d = (fragCoord.x / iResolution.y) * 10.0;
            float phase = (d + fract(t * frequency) + phaseShiftPerSource * float(i)) * PI2;
            float sp = sin(phase);
            float dy = (abs(1.0 - (((fragCoord.y / iResolution.y) * 50.0 - 1.5 - float(i) * 3.0) - sin(phase))) - 0.02) * 30.0;
            overlay += vec4(0., 0., 0., clamp(1.0 - dy, 0., 1.) ) * syncBlend;
        }
    
    float power = sqrt(sumSin * sumSin + sumCos * sumCos) * powerScale;
    //power *= power;
    vec4 field = mix(
        vec4(sat(sumSin * powerScale), 0, sat(-sumSin * powerScale), 1.0), 
        vec4(power, power, power, 1.0),
            powerDisplayValue);
    fragColor = vec4(
        sqrt(mix(field.r * field.r, overlay.r * overlay.r, overlay.a)),
        sqrt(mix(field.g * field.g, overlay.g * overlay.g, overlay.a)),
        sqrt(mix(field.b * field.b, overlay.b * overlay.b, overlay.a)),
            1.0);
}