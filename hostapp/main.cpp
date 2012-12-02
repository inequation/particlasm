/* particlasm demo app
 * Based on NeHe OpenGL lesson #19
 * Leszek Godlewski <github@inequation.org>
 *
 * This code was created by Jeff Molofee '99
 * (ported to Linux/SDL by Ti Leggett '01)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 *
 * or for port-specific comments, questions, bugreports etc.
 * email to leggett@eecs.tulane.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <ctime>

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <time.h>
	#define PLATFORM		"windows"
	#define PATH_SEPARATOR	"\\"
	#define LOCAL_PATH
	#define SO_EXT			".dll"
	typedef HMODULE			SO_HANDLE;
	#define dlopen(a, b)	LoadLibrary(a)
	#define dlsym(a, b)		GetProcAddress(a, b)
	#define dlclose(a)		FreeLibrary(a)
#else
	#include <unistd.h>
	#include <dlfcn.h>
	#include <sys/time.h>
	#define PLATFORM		"linux"
	#define PATH_SEPARATOR	"/"
	#define LOCAL_PATH		"./"
	#define SO_EXT			".so"
	typedef void			*SO_HANDLE;
	// change this to 0 if your compiler complains about dladdr
	#define HAVE_DLADDR		1
#endif // WIN32

#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
	#define ARCH			"x64"
	#define LOCAL_TARGET	ptcTarget_x86_64
#else
	#define ARCH			"x86"
	#define LOCAL_TARGET	ptcTarget_x86
#endif

// particlasm functions
#include <libparticlasm2.h>
PFNPTCGETAPI	ptcGetAPI;
ptcAPIExports	ptcAPI;

//#define USE_CPP_REFERENCE_IMPLEMENTATION

#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
extern "C" PTC_ATTRIBS unsigned int ref_ptcCompileEmitter(ptcEmitter *emitter);
extern "C" PTC_ATTRIBS uint32_t ref_ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);
extern "C" PTC_ATTRIBS void ref_ptcReleaseEmitter(ptcEmitter *emitter);
#else
void *libparticlasmHandle = NULL;
#endif // USE_CPP_REFERENCE_IMPLEMENTATION

bool InitParticlasm() {
#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
	ptcAPI.CompileEmitter = ref_ptcCompileEmitter;
	ptcAPI.ProcessEmitter = ref_ptcProcessEmitter;
	ptcAPI.ReleaseEmitter = ref_ptcReleaseEmitter;
	return true;
#else
	libparticlasmHandle = dlopen(LOCAL_PATH "libparticlasm2-" PLATFORM "-" ARCH SO_EXT, RTLD_NOW);
	if (!libparticlasmHandle) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
		TCHAR *errStr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
			LANG_USER_DEFAULT, (LPSTR)&errStr, 0, NULL);
		printf("GetLastError: %s\n", errStr);
		LocalFree((HLOCAL)errStr);
#else
		printf("dlerror: %s\n", dlerror());
#endif
		return false;
	}
	ptcGetAPI = (PFNPTCGETAPI)dlsym(libparticlasmHandle, PTC_ENTRY_POINT);
	if (ptcGetAPI && ptcGetAPI(PTC_API_VERSION, &ptcAPI)
		&& ptcAPI.InitializeTarget(LOCAL_TARGET, NULL))
		return true;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
	TCHAR *errStr;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
		LANG_USER_DEFAULT, (LPSTR)&errStr, 0, NULL);
	printf("GetLastError: %s\n", errStr);
	LocalFree((HLOCAL)errStr);
#else
	printf("dlerror: %s\n", dlerror());
#endif
	dlclose(libparticlasmHandle);
	return false;
#endif // USE_CPP_REFERENCE_IMPLEMENTATION
}

void FreeParticlasm() {
#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
#else
	if (libparticlasmHandle) {
		ptcAPI.ShutdownTarget(NULL);
		dlclose(libparticlasmHandle);
	}
#endif // USE_CPP_REFERENCE_IMPLEMENTATION
}

/* screen width, height, and bit depth */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     16

/* Setup our booleans */
#define TRUE  1
#define FALSE 0

/* Max number of particles */
#define MAX_PARTICLES 8000

/* This is our SDL surface */
SDL_Surface *surface;

float zoom = 100.0f;   /* Used To Zoom Out                                   */
float timescale = 1.f;
size_t ptc_nemitters;
ptcEmitter *ptc_emitters;
volatile size_t ptc_nvertices;
ptcParticle ptc_particles[MAX_PARTICLES];
ptcVertex ptc_vertices[sizeof(ptc_particles) / sizeof(ptc_particles[0]) * 4];

extern "C" size_t Fire(ptcEmitter **emitters);

GLuint texture[1];     /* Storage For Our Particle Texture                   */

int grid = TRUE;       /* Toggle grid drawing                                */

float pitch;
float yaw;

static GLint prev_t = 0;

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
	for (size_t i = 0; i < ptc_nemitters; ++i)
		ptcAPI.ReleaseEmitter(&ptc_emitters[i]);
	// clean up particlasm
	FreeParticlasm();

    /* Clean up our textures */
    glDeleteTextures( 1, &texture[0] );

    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}

/* function to load in bitmap as a GL texture */
int LoadGLTextures( )
{
    /* Status indicator */
    int Status = FALSE;

    /* Create storage space for the texture */
    SDL_Surface *TextureImage[1];

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage[0] = SDL_LoadBMP( "data/particle.bmp" ) ) )
        {

	    /* Set the status to true */
	    Status = TRUE;

	    /* Create The Texture */
	    glGenTextures( 1, &texture[0] );

	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w,
			  TextureImage[0]->h, 0, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }

    /* Free up any memory we may have used */
    if ( TextureImage[0] )
	    SDL_FreeSurface( TextureImage[0] );

    return Status;
}


/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;

    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )width, ( GLint )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 1000.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    return( TRUE );
}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    /* ESC key was pressed */
	    Quit( 0 );
	    break;
	case SDLK_F1:
	    /* F1 key was pressed
	     * this toggles fullscreen mode
	     */
	    SDL_WM_ToggleFullScreen( surface );
	    break;
	case SDLK_EQUALS:
	    /* '+' key was pressed
	     * this speeds up the particles
	     */
	    timescale += 0.05;
	    if (timescale > 10.f)
			timescale = 10.f;
		printf("timescale: %3.2f\n", timescale);
	    break;
	case SDLK_MINUS:
	    /* '-' key was pressed
	     * this slows down the particles
	     */
	    timescale -= 0.05;
	    if (timescale < 0.01)
			timescale = 0.01;
		printf("timescale: %3.2f\n", timescale);
		break;
	case SDLK_PAGEUP:
	    /* PageUp key was pressed
	     * this zooms into the scene
	     */
	    zoom += 1.f;
	    if (zoom > 1000.f)
			zoom = 1000.f;
		printf("zoom: %3.2f\n", zoom);
	    break;
	case SDLK_PAGEDOWN:
	    /* PageDown key was pressed
	     * this zooms out of the scene
	     */
	    zoom -= 1.f;
	    if (zoom < 1.f)
			zoom = 1.f;
		printf("zoom: %3.2f\n", zoom);
	    break;
	case SDLK_g:
		grid = !grid;
		break;
	case SDLK_r:
		// reset camera
		pitch = 0;
		yaw = 0;
		break;
	default:
	    break;
	}

    return;
}

/* general OpenGL initialization function */
int initGL(void)
{
	if (glewInit() != GLEW_OK)
		return FALSE;

	if (!GLEW_ARB_point_sprite || !GLEW_ARB_point_parameters
		|| !glPointParameterfvARB)
		return FALSE;

    /* Load in the texture */
    if ( !LoadGLTextures( ) )
		return FALSE;

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glDisable( GL_DEPTH_TEST );

    /* Enable Blending */
    glEnable( GL_BLEND );
    /* Type Of Blending To Perform */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    /* Really Nice Point Smoothing */
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
    /* Really Nice Line Smoothing */
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    /* Enable Texture Mapping */
    glEnable( GL_TEXTURE_2D );
    /* Select Our Texture */
    glBindTexture( GL_TEXTURE_2D, texture[0] );

	ptc_nemitters = Fire(&ptc_emitters);
	for (size_t i = 0; i < ptc_nemitters; ++i) {
		ptc_emitters[i].ParticleBuf = ptc_particles;
		ptc_emitters[i].MaxParticles = sizeof(ptc_particles) / sizeof(ptc_particles[0]);
		if (!ptcAPI.CompileEmitter(&ptc_emitters[i]))
			return FALSE;
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(ptcVertex), &ptc_vertices[0].Location[0]);
	glTexCoordPointer(2, GL_SHORT, sizeof(ptcVertex), &ptc_vertices[0].TexCoords[0]);
	glColorPointer(4, GL_FLOAT, sizeof(ptcVertex), &ptc_vertices[0].Colour[0]);

    return( TRUE );
}

#define GRID_SIZE	20

/* Here goes our drawing code */
int drawGLScene(void)
{
    /* These are to calculate our fps */
    static GLint T0     = 0;
    static GLint Frames = 0;

    static ptcVector cameraCS[3];

    GLint t = SDL_GetTicks();
    float frameTime = (t - prev_t) * 0.001 * timescale;
    prev_t = t;

    /* Clear The Screen And The Depth Buffer */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glLoadIdentity( );

    // orbit around the origin
    glTranslatef(0.f, 0.f, -zoom);
    glRotatef(-pitch, 1.f, 0.f, 0.f);
    glRotatef(yaw, 0.f, 1.f, 0.f);

	// get camera space vectors
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    cameraCS[0][0] = -mat[2];
    cameraCS[0][1] = -mat[6];
    cameraCS[0][2] = -mat[10];
    cameraCS[1][0] = mat[0];
    cameraCS[1][1] = mat[4];
    cameraCS[1][2] = mat[8];
    cameraCS[2][0] = mat[1];
    cameraCS[2][1] = mat[5];
    cameraCS[2][2] = mat[9];

    /* Draw the grid */
    if (grid)
	{
		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINES );
		glColor4f(0.5, 0.5, 0.5, 1.f);
		// lines down
		for ( int i = 0; i < GRID_SIZE + 1; ++i )
		{
			glVertex3i(10 * (-GRID_SIZE / 2 + i), 0, 10 * -GRID_SIZE / 2);
			glVertex3i(10 * (-GRID_SIZE / 2 + i), 0, 10 * GRID_SIZE / 2);
		}
		// and across
		for ( int i = 0; i < GRID_SIZE + 1; ++i )
		{
			glVertex3i(10 * -GRID_SIZE / 2, 0, 10 * (-GRID_SIZE / 2 + i));
			glVertex3i(10 * GRID_SIZE / 2, 0, 10 * (-GRID_SIZE / 2 + i));
		}
		// coordinate system gizmo
		glColor4f(1.f, 0.f, 0.f, 1.f);
		glVertex3i(0, 0, 0);
		glVertex3i(10, 0, 0);
		glColor4f(0.f, 1.f, 0.f, 1.f);
		glVertex3i(0, 0, 0);
		glVertex3i(0, 10, 0);
		glColor4f(0.f, 0.f, 1.f, 1.f);
		glVertex3i(0, 0, 0);
		glVertex3i(0, 0, 10);

		glEnd();
		glEnable( GL_TEXTURE_2D );
	}

	ptc_nvertices = 0;
	for (size_t i = 0; i < ptc_nemitters; ++i)
	{
		ptc_nvertices += ptcAPI.ProcessEmitter(&ptc_emitters[i], frameTime,
			cameraCS, ptc_vertices + ptc_nvertices,
			sizeof(ptc_vertices) / sizeof(ptc_vertices[0]) - ptc_nvertices);
	}
	//glDrawArrays(GL_QUADS, 0, 4);//ptc_nvertices);
	glBegin(GL_QUADS);
	for (size_t i = 0; i < ptc_nvertices; ++i) {
		glColor4fv(ptc_vertices[i].Colour);
		glTexCoord2sv(ptc_vertices[i].TexCoords);
		glVertex3fv(ptc_vertices[i].Location);
	}
	glEnd();

    /* Draw it to the screen */
    SDL_GL_SwapBuffers( );

    /* Gather our frames per second */
    Frames++;
    {
		if (t - T0 >= 1000) {
			float seconds = (t - T0) / 1000.0;
			float fps = Frames / seconds;
			printf("%d frames in %.3g seconds = %.3g FPS, %lu particles, %lu vertices\n",
				Frames, seconds, fps, ptc_emitters[0].NumParticles, ptc_nvertices);
			T0 = t;
			Frames = 0;
		}
    }

    return( TRUE );
}

int main( int argc, char **argv )
{
    /* Flags to pass to SDL_SetVideoMode */
    int videoFlags;
    /* main loop variable */
    int done = FALSE;
    /* used to collect events */
    SDL_Event event;
    /* this holds some info about our display */
    const SDL_VideoInfo *videoInfo;
    /* whether or not the window is active */
    int isActive = TRUE;

	// initialize random number generator
	srand((unsigned int)time(NULL));

    /* initialize SDL */
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* Fetch the video info */
    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* the flags to pass to SDL_SetVideoMode                            */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL          */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering       */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing        */

    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
	videoFlags |= SDL_HWSURFACE;
    else
	videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
	videoFlags |= SDL_HWACCEL;

    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /* get a SDL surface */
    surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
				videoFlags );

    /* Verify there is a surface */
    if ( !surface )
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    Quit( 1 );
	}

    /* Enable key repeat */
    if ( ( SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL ) ) )
	{
	    fprintf( stderr, "Setting keyboard repeat failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

	if (!InitParticlasm())
	{
		fprintf(stderr, "Could not initialize particlasm.\n");
		Quit(1);
	}

    /* initialize OpenGL */
    if ( initGL( ) == FALSE )
	{
	    fprintf( stderr, "Could not initialize OpenGL.\n" );
	    Quit( 1 );
	}

    /* Resize the initial window */
    resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );

    /* wait for events */
    while ( !done )
	{
	    /* handle the events in the queue */

	    while ( SDL_PollEvent( &event ) )
		{
		    switch( event.type )
			{
			case SDL_ACTIVEEVENT:
			    /* Something's happend with our focus
			     * If we lost focus or we are iconified, we
			     * shouldn't draw the screen
			     */
			    if ( event.active.gain == 0 )
				isActive = FALSE;
			    else
				isActive = TRUE;
			    break;
			case SDL_VIDEORESIZE:
			    /* handle resize event */
			    surface = SDL_SetVideoMode( event.resize.w,
							event.resize.h,
							16, videoFlags );
			    if ( !surface )
				{
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    Quit( 1 );
				}
			    resizeWindow( event.resize.w, event.resize.h );
			    break;
			case SDL_KEYDOWN:
			    /* handle key presses */
			    handleKeyPress( &event.key.keysym );
			    break;
			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON(1))
				{
					zoom += event.motion.yrel * 0.5;
					if (zoom < 1.f)
						zoom = 1.f;
					else if (zoom > 1000.f)
						zoom = 1000.f;
					printf("zoom: %3.2f\n", zoom);
				}
				else if (event.motion.state & SDL_BUTTON(3))
				{
					float zoomMult = 0.5 + (zoom - 1.f) / (1000.f - 1.f);
					yaw -= event.motion.xrel * zoomMult;
					if (yaw > 180.f)
						yaw -= 360.f;
					else if (yaw < -180.f)
						yaw += 360.f;
					pitch -= event.motion.yrel * zoomMult;
					if (pitch > 90.f)
						pitch = 90.f;
					else if (pitch < -90.f)
						pitch = -90.f;
				}
				break;
			case SDL_QUIT:
			    /* handle quit requests */
			    done = TRUE;
			    break;
			default:
			    break;
			}
		}

	    /* draw the scene */
	    if ( isActive )
			drawGLScene( );
		else
			// fake last frame time so that we pause simulation instead of dropping frames
			prev_t = SDL_GetTicks();
	}

    /* clean ourselves up and exit */
    Quit( 0 );

    /* Should never get here */
    return( 0 );
}
