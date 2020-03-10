#version 300 es
precision highp float;
precision highp sampler3D;

out vec4 outColor;

uniform sampler3D noise_tex;
uniform vec3 ro;
uniform mat3 camera;
uniform vec2 iResolution;
uniform vec3 iVoxelDataCenter;// center of the voxel grid in world space units
uniform float iVoxelDataSize; // voxel half-extent in world space units
uniform float iVoxelDataSize_rcp; 

uniform mat4 iModelTransform;

const vec3 forward = vec3(0.f, 0.f, 1.f);
const vec3 up = vec3(0.f, 1.f, 0.f);
const vec3 right = vec3(-1.f, 0.f, 0.f);

// Common function

float hmax(in vec2 a) { return max(a.x, a.y); }
float hmax(in vec3 a) { return hmax(vec2(a.x, hmax(a.yz))); }
float hmax(in vec4 a) { return hmax(vec2(hmax(a.xy), hmax(a.zw))); }

float hmin(in vec2 a) { return min(a.x, a.y); }
float hmin(in vec3 a) { return hmin(vec2(a.x, hmin(a.yz))); }
float hmin(in vec4 a) { return hmin(vec2(hmin(a.xy), hmin(a.zw))); }

float saturate(in float a) { return clamp(a, 0.0, 1.0); }
vec2 saturate(in vec2 a) { return vec2(saturate(a.x), saturate(a.y)); }
vec3 saturate(in vec3 a) { return vec3(saturate(a.x), saturate(a.y), saturate(a.z)); }
vec4 saturate(in vec4 a) { return vec4(saturate(a.x), saturate(a.y), saturate(a.z), saturate(a.w)); }

bool is_saturated(in float a) { return a == saturate(a); }
bool is_saturated(in vec2 a) { return is_saturated(a.x) && is_saturated(a.y); }
bool is_saturated(in vec3 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z); }
bool is_saturated(in vec4 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z) && is_saturated(a.w); }

mat3 setCamera( in vec3 ro, in vec3 ta, in float cr )
{
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, -cw );
}

// o: origin
// d: normalized direction
// d_rcp = 1.0 / d
struct Ray {
 	vec3 o;
    vec3 d;
    vec3 d_rcp;
};

// t: distance to impact
// id: entity id hit
struct RayHit {
    float t;
    int id;
};
    
Ray makeRay(in vec3 origin, in vec3 direction)
{
    vec3 d = normalize(direction);
    return Ray(origin, d, vec3(1.0) / d);
}

RayHit makeRayHit()
{
	return RayHit(-1.0, -1);
}

RayHit getClosest(in RayHit r1, in RayHit r2)
{
    RayHit r;
    bool r1valid = 0 <= r1.id;
    bool r2valid = 0 <= r2.id;
    if (r1valid && r2valid)
    {
        if(r1.t < r2.t)
        {
            r = r1;
        }
        else
        {
            r = r2;
        }
    }
    else if (r1valid)
    {
        r = r1;
    }
    else
    {
        r = r2;
    }  
    return r;
}

// p: position in box space
// b: box half-extent
// return distance from box boundaries (negative if inside)
float sdBox( in vec3 p, in vec3 b )
{
    vec3 d = abs(p) - b;
    return min(hmax(d),0.0) + length(max(d,0.0));
}

// r: ray in box space
// halfExtent: box half-extent
// t0: distance to first intersection
// t1: distance to second intersection
// return an intersection exist
bool intersectBox( in Ray r, in vec3 halfExtent, inout float t0, inout float t1 )
{
    vec3 tbot = r.d_rcp * (-halfExtent - r.o);
    vec3 ttop = r.d_rcp * (halfExtent - r.o);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    t0 = max(t0, hmax(tmin));
    t1 = min(t1, hmin(tmax));
    return t0 <= t1;
}

struct Plane {
	vec3 p;
	vec3 n;
};

bool intersectPlane( in Ray r, in Plane plane, out float t) 
{
    t = -1.0;
    vec3 n = -plane.n;
    float denom = dot(n, r.d); 
    if (denom > 1e-6) { 
        vec3 pro = plane.p - r.o; 
        t = dot(pro, n) / denom; 
        return (t >= 0.0); 
    }
    return false; 
} 

vec3 gamma(in vec3 col)
{
    return pow( saturate(col), vec3(0.45) );
}

// end common function

const float iTime = 1.0;
#define AA 1   // make this 2 or 3 for antialiasing
#define SHADOW 0

const float g_VoxelDataRayStepSize = 1.0 ;
const float g_VoxelDataRadius = 0.001;

const vec3 g_lightDirection = normalize( vec3(0.4, 0.3, -0.3) );

const int g_RayCastMaxStep = 256;
const float g_RayCastMaxDistance = 10.0;

const Plane g_plane = Plane(vec3(0.0, -1.0, 0.0), vec3(0.0, 1.0, 0.0));

const int ePlaneId = 1;
const int eVoxelId = 2;

vec3 projectToTextureCoord(in vec3 worldCoord)
{
	vec3 tc = worldCoord;
	tc = (tc - iVoxelDataCenter) * iVoxelDataSize_rcp;
	tc = tc * vec3(0.5f, -0.5f, 0.5f) + 0.5f;
	return tc;
}

float voxelDistance( in vec3 worldCoord )
{
	vec3 voxelDataSize = vec3(iVoxelDataSize);
    float td_box = max(0.0, sdBox( worldCoord - iVoxelDataCenter, voxelDataSize ));
    vec3 clamped_worldCoord = clamp(worldCoord, -voxelDataSize, voxelDataSize); // bad idea to clamp?
    // Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling
    vec3 tc = projectToTextureCoord(clamped_worldCoord);
    float tcol = texture(noise_tex, tc).x;
    float td = tcol;
    // Do add negative value oustide the box
    td = clamped_worldCoord != worldCoord ? max(0.0, td): td;
    return td + td_box;
}

// r: world-space ray
// return: distance to impact (negative if no hit)
RayHit castRay( in Ray r, out int stepCount )
{
	RayHit hVoxel = makeRayHit();
    RayHit hPlane = makeRayHit();
    
    // test the plane
    if(intersectPlane(r, g_plane, hPlane.t))
    {
        hPlane.id = ePlaneId;
    }
    
    stepCount = 0;
    float t = 0.0;
    float tmax = g_RayCastMaxDistance;
	vec3 startPos = r.o;
	vec3 rayDirection = r.d;
    vec3 boxSize = vec3(iVoxelDataSize);
    
    // advance on the box
    Ray rbox = Ray(r.o - iVoxelDataCenter, r.d, r.d_rcp);
    if(!intersectBox(rbox, boxSize, t, tmax))
    {
		return hPlane;
    }
	
	// try to advance on the box
	const float epsilon = 0.001;
    t += epsilon;
	
	for(stepCount = 0; stepCount < g_RayCastMaxStep; ++stepCount)
	{
        float td = voxelDistance(r.o + r.d * t) - g_VoxelDataRadius;
        if (g_VoxelDataRadius <= td && td <= g_VoxelDataRadius * 10.0)
            break;
        
		t += td * g_VoxelDataRayStepSize;
	}
	
	if ( t <= tmax )
    {
		hVoxel.id = eVoxelId;
        hVoxel.t = t;
    }
	
	return getClosest(hPlane, hVoxel);
}

// pos: world-space pos
// id: entity id
vec3 calcNormal( in vec3 pos, in int id )
{
    vec3 n = vec3(0.0);
    if( ePlaneId == id )
    {
        n = g_plane.n;
    }
    else if( eVoxelId == id )
    {
        // @credit https://www.shadertoy.com/view/Xds3zN
        for( int i=0; i<4; i++ )
        {
            // TODO this should depend on the voxel size and resolution
            vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
            n += e*voxelDistance(pos+0.05*e);
        }
        n = normalize(n);
    }
    return n;
}

// @credit https://www.shadertoy.com/view/lsKcDD
float calcSoftshadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
#if SHADOW < 1
    return 1.0;
#endif
    float res = 1.0;
    float t = mint;
    float ph = 1e10; // big, such that y = 0 on the first iteration
    
    for( int i=0; i<32; i++ )
    {
		float h = voxelDistance(ro + rd*t);

#if SHADOW == 1
        // traditional technique
        res = min( res, 10.0*h/t );
#else
        // improved technique
        // use this if you are getting artifact on the first iteration, or unroll the
        // first iteration out of the loop
        // float y = (i==0) ? 0.0 : h*h/(2.0*ph); 

        float y = h*h/(2.0*ph);
        float d = sqrt(h*h-y*y);
        res = min( res, 10.0*d/max(0.0,t-y) );
        ph = h;
#endif
        t += h;
        
        if( res<0.0001 || t>tmax ) break;
        
    }
    return clamp( res, 0.0, 1.0 );
}

// @credit https://www.shadertoy.com/view/lsKcDD
float calcAO( in vec3 pos, in vec3 nor )
{
#if SHADOW < 1
    return 1.0;
#endif
    float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float h = 0.001 + 0.15*float(i)/4.0;
        float d = voxelDistance( pos + h*nor );
        occ += (h-d)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 1.5*occ, 0.0, 1.0 );    
}

vec3 lighting( in Ray r, in RayHit h, in int stepCount)
{
    vec3 col = vec3(0.0);
    
#if 1
    // @credit https://www.shadertoy.com/view/lsKcDD
    float t = h.t;
    vec3 pos = r.o + t*r.d;
    vec3 nor = calcNormal( pos, h.id );
    
    // material        
    vec3 mate = vec3(0.3);

    // key light
    vec3  lig = g_lightDirection;
    vec3 hal = normalize( lig-r.d );
    float dif = clamp( dot( nor, lig ), 0.0, 1.0 ) * calcSoftshadow( pos, lig, 0.01, 3.0 );
    float spe = pow( clamp( dot( nor, hal ), 0.0, 1.0 ),16.0)*
        dif *
        (0.04 + 0.96*pow( clamp(1.0+dot(hal,r.d),0.0,1.0), 5.0 ));

    col = mate * 4.0*dif*vec3(1.00,0.70,0.5);
    col +=      12.0*spe*vec3(1.00,0.70,0.5);

    // ambient light
    float occ = calcAO( pos, nor );
    float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
    col += mate*amb*occ*vec3(0.0,0.08,0.1);

    // fog
    col *= exp( -0.0005*t*t*t );
    return col;
#endif
    
    if( ePlaneId == h.id )
    {
        col = vec3(0.5, 0.5, 0.5);
    }
    else if( eVoxelId == h.id )
    {
#if 1
        // Draw Distance
        float t = saturate( h.t / g_RayCastMaxDistance);
        vec3 near = vec3(1.0,0.0,0.0);
        vec3 far = vec3(0.0,1.0,0.0);
        col = mix(near,far,t);
#else 
#if 0
        // Draw step count
        vec3 low = vec3(0.0,0.0,1.0);
        vec3 mid = vec3(0.0,1.0,0.0);
        vec3 high = vec3(1.0,0.0,0.0);
        if (stepCount < 15)
            col = low;
        else if (stepCount < 30)
            col = mid;
        else
            col = high;
#else
        // Draw texture
        vec3 tc = projectToTextureCoord(r.o + r.d * h.t);
        float tcol = texture(iChannel0, tc).x;
        col = vec3(tcol);
#endif
#endif  
    }
    return col;
}

vec3 render( in Ray r)
{
	int stepCount = 0;
	RayHit h = castRay(r, stepCount);
    vec3 col = lighting( r, h, stepCount );
	return col;
}

void main()
{  
    // camera-to-world transformation
    mat3 ca = camera;
    
    // animation
    float lightSize = 0.05 + 0.04*sin(0.7*iTime);

    // render
    vec3 tot = vec3(0.0);
#if AA>1
    for( int m=0; m<AA; m++ )
    for( int n=0; n<AA; n++ )
    {
        // pixel coordinates
        vec2 o = vec2(float(m),float(n)) / float(AA) - 0.5;
        vec2 p = -1.0 + 2.0*(gl_FragCoord.xy + o) / iResolution.xy;
#else    
        vec2 p = -1.0 + 2.0*(gl_FragCoord.xy) / iResolution.xy;
#endif
        p.x *= -iResolution.x / iResolution.y;

        // ray direction
        vec3 rd = ca * normalize( vec3(p,2.0) );

        // render	
        vec3 col = render( makeRay(ro, rd) );

        col = gamma(col);

        tot += col;
#if AA>1
    }
    tot /= float(AA*AA);
#endif

    outColor = vec4( tot, 1.0);
}
