#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

uniform float refractiveIndex;
uniform float fresnelBase;          
uniform float dispersionStrength;

uniform bool useDispersion;
uniform bool useFresnel;
uniform bool useRefraction;
uniform bool useReflection;

float fresnelSchlick(float cosTheta, float F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 I = normalize(Position - cameraPos);


    vec3 reflectedColor = vec3(0.0);
    if (useReflection) {
        vec3 R = reflect(I, normal);
        reflectedColor = texture(skybox, R).rgb;
    }


    vec3 refractedColor = vec3(0.0);
    if (useRefraction) {

        if (useDispersion) {

            float iorR = max(1.0, refractiveIndex - dispersionStrength);
            float iorG = max(1.0, refractiveIndex);
            float iorB = max(1.0, refractiveIndex + dispersionStrength);

            vec3 dirR = refract(I, normal, 1.0 / iorR);
            vec3 dirG = refract(I, normal, 1.0 / iorG);
            vec3 dirB = refract(I, normal, 1.0 / iorB);

            vec3 colR = texture(skybox, dirR).rgb;
            vec3 colG = texture(skybox, dirG).rgb;
            vec3 colB = texture(skybox, dirB).rgb;

            // Combine channels to create the dispersion split
            refractedColor = vec3(colR.r, colG.g, colB.b);
        }
        else {
            vec3 refrDir = refract(I, normal, 1.0 / refractiveIndex);
            refractedColor = texture(skybox, refrDir).rgb;
        }
    }


    float cosTheta = clamp(dot(-I, normal), 0.0, 1.0);
    float fresnelFactor = useFresnel ? fresnelSchlick(cosTheta, fresnelBase) : 0.5;

    vec3 finalColor;
    if (useReflection && !useRefraction) finalColor = reflectedColor;
    else if (!useReflection && useRefraction) finalColor = refractedColor;
    else if (!useReflection && !useRefraction) finalColor = vec3(0.0);
    else finalColor = mix(refractedColor, reflectedColor, fresnelFactor);

    FragColor = vec4(finalColor, 1.0);
}

