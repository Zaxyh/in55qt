#version 330

in vec4 fragPosWorld;
in vec4 fragNorWorld;
in vec2 fragST;

uniform vec4 eyePosWorld;
uniform vec4 lightPosWorld;
uniform vec4 lightColor;

uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;

uniform vec4 ambient; // Global ambient contribution.

uniform sampler2D texture;

out vec4 fragColor;

void main()
{
    // Compute the diffuse term.
    vec4 N = normalize( fragNorWorld );
    vec4 L = normalize( lightPosWorld - fragPosWorld );
    float NdotL = max( dot( N, L ), 0 );
    vec4 diffuse =  NdotL * lightColor * materialDiffuse;

    // Compute the specular term.
    vec4 V = normalize( eyePosWorld - fragPosWorld );
    vec4 H = normalize( L + V );
    vec4 R = reflect( -L, N );
    float RdotV = max( dot( R, V ), 0 );
    float NdotH = max( dot( N, H ), 0 );
    vec4 specular = pow( RdotV, materialShininess ) * lightColor * materialSpecular;

    fragColor = ( ambient + diffuse + specular ) * texture2D( texture, fragST );
}
