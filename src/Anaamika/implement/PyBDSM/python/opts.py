"""PyBDSM options

Options are essentially user-controllable parameters passed into PyBDSM
operations, and allow for end-users to control the exact details of how
calculations are done.

The doc string should give a short description of the option, followed by a
line break ('\n') then a long, detailed description. The short description can
then be split off using "str(v.doc()).split('\n')[0]".

The group string can be used to group suboptions under a parent option.  The
group string should be the name of the parent option, which must be Bool
(except for the "hidden" group, which will suppress listing of the option; the
option can still be set as normal).

In general it's better to specify newly added options directly in this file, so
one can oversee them all. But it's also possible to extend it at run-time, and
under some circumstances (e.g. pybdsm installed system-wide, and there is no
way to modify this file) this might be the only option to do so. An example of
such extension follows:

==== file newmodule.py ====
from image import Op

class Op_new_op(Op):
    ## do something useful here
    ## we need to add option my_new_opt
    pass

## this will extend Opts class at runtime and ensure that 
## type-checking works properly.
Opts.my_new_opt = Float(33, doc="docstring")
"""
import sys
from tc import Int, Float, Bool, String, Tuple, Enum, \
    Option, NArray, Instance, tInstance, List, Any, TCInit, tcError

class Opts(object):
    """Class Opts -- user-controllable parameters."""
    advanced_opts   =   Bool(False,
                             doc="Show advanced options")
    atrous_do       =   Bool(False,
                             doc="Decompose Gaussian residual image "\
                                 "into multiple scales\n"\
                                 "If True, then the Gaussian-subtracted "\
                                 "residual image is decomposed into multiple "\
                                 "scales using an a-trous wavelet transform.")
    beam            = Option(None, Tuple(Float(), Float(), Float()),
                             doc="FWHM of restoring beam. Specify as (maj, "\
                                 "min, pos ang E of N) in degrees. "\
                                 "E.g., beam = (0.06, 0.02, 13.3). None => "\
                                 "get from header\n"\
                                 "If the beam is not given "\
                                 "by the user, then it is looked for in the "\
                                 "image header. If not found, then an error "\
                                 "is raised. PyBDSM will not work without the "\
                                 "knowledge of the restoring beam.")                             
    filename        = String(doc="Input image file name\n"\
                                 "If the filename ends in '_I', then similar files "\
                                 "with '_Q', '_U' and '_V' are looked for, and "\
                                 "if not present, the parameter polarisation_do "\
                                 "is set to False. Eventually, PyBDSM will be able "\
                                 "to handle 4-D cubes and this convention will "\
                                 "no longer be necessary.")
    flagging_opts   =   Bool(False,
                             doc="Show options for Gaussian flagging")
    frequency       = Option(None, Float(),
                             doc="Frequency in Hz of input image. "\
                                 "E.g., frequency = 74e6. None => get from header. "\
                                 "For more than one channel, use the frequency_sp "\
                                 "parameter.\nIf the frequency is not given "\
                                 "by the user, then it is looked for in the "\
                                 "image header. If not found, then an error "\
                                 "is raised. PyBDSM will not work without the "\
                                 "knowledge of the frequency.")
    interactive     =   Bool(False,
                             doc="Use interactive mode\n"\
                                 "In interactive mode, plots are displayed at "\
                                 "various stages of the processing so that "\
                                 "the user may check the progress of the fit.\n"\
                                 "First, plots of the rms and mean background images are "\
                                 "displayed along with the islands found, before "\
                                 "fitting of Gaussians takes place. The user should "\
                                 "verify that the islands and maps are reasonable "\
                                 "before preceding.\n"\
                                 "Next, if atrous_do is True, the fits to each "\
                                 "wavelet scale are shown. The wavelet fitting "\
                                 "may be truncated at the current scale if "\
                                 "desired.\nLastly, the final results are shown.")
    mean_map        =   Enum('default', 'zero', 'const', 'map',
                             doc="Background mean map: 'default' => calc whether "\
                                 "to use or not, 'zero' => 0, 'const' => "\
                                 "clipped mean, 'map' => use 2-D map.\n"\
                                 "This parameter determines "\
                                 "how the background mean map is computed "\
                                 "and how it is used further.\nIf 'const', then "\
                                 "the value of the clipped "\
                                 "mean of the entire image (set by the kappa_clip "\
                                 "option) is used as the "\
                                 "background mean map.\nIf 'zero', then a value "\
                                 "of zero is used.\nIf 'map', then "\
                                 "the 2-dimensional mean map is computed and used. "\
                                 "The resulting mean map is largely determined by "\
                                 "the value of the rms_box parameter (see the "\
                                 "rms_box parameter for more information).\nIf "\
                                 "'default', then PyBDSM will attempt to "\
                                 "determine automatically whether to use "\
                                 "a 2-dimensional map or a constant one as "\
                                 "follows. First, "\
                                 "the image is assumed to be confused if "\
                                 "bmpersrc_th < 25 or the ratio of the "\
                                 "clipped mean to rms (clipped mean/clipped rms) "\
                                 "is > 0.1, else the image is not confused. "\
                                 "Next, the mean map is checked to "\
                                 "see if its spatial variation is significant. If "\
                                 "so, then a 2-D map is used and, if not, "\
                                 "then the mean map is set to either 0.0 or a "\
                                 "constant depending on whether the image is "\
                                 "thought to be confused or not.")
    multichan_opts  =   Bool(False,
                             doc="Show options for multi-channel "\
                                 "images")
    output_opts     =   Bool(False,
                             doc="Show output options")
    polarisation_do =   Bool(False,
                             doc="Find polarisation properties\n"\
                                 "Finds the flux of sources detected in the Stokes I "\
                                 "image in each of the other Stokes images and calculates the "\
                                 "polarisation fraction and angle for each source.\n"\
                                 "If True, then polarisation properties -- polarisation "\
                                 "fraction (linear, circular, total) and polarisation "\
                                 "angle -- are determined. These parameters are estimated "\
                                 "only for sources which are found in the Stokes I image. "\
                                 "Currently, each Stokes image has to be in a "\
                                 "separate file, with names of the form 'filename_I.ext', "\
                                 "'filename_Q.ext', 'filename_U.ext', and 'filename_V.ext'.\n"\
                                 "Fluxes are calculated by summing all nonmasked "\
                                 "pixels assigned to the source. If a pixel contains "\
                                 "contributions from two or more sources, its flux is "\
                                 "assigned to the source with the largest contribution. "\
                                 "Errors on the fluxes are derived by summing the same "\
                                 "pixels in the rms maps in quadrature.\n"\
                                 "For linearly polarised emission, the signal and noise "\
                                 "add vectorially, giving a Rice distribution "\
                                 "(Vinokur 1965) instead of a Gaussian one. To correct "\
                                 "for this, a bias is estimated and removed from the "\
                                 "polarisation fraction using the same method used for the "\
                                 "NVSS catalog (see ftp://ftp.cv.nrao.edu/pub/nvss/catalog.ps). "\
                                 "Errors on the linear and total polarisation fractions "\
                                 "and polarisation angle are estimated using the debiased "\
                                 "polarised flux and standard error propagation. See "\
                                 "Sparks & Axon (1999) for a more detailed treatment.")
    psf_vary_do     =   Bool(False,
                             doc="Calculate PSF variation across image")
    rm_do           =   Bool(False,
                             doc="Find rotation measure properties",
                             group='hidden')
    rms_box         = Option(None, Tuple(Int(), Int()),
                             doc="Box size, step size for rms/mean map "\
                                 "calculation. Specify as (box, step) in "\
                                 "pixels. E.g., rms_box = (40, 10) => box "\
                                 "of 40x40 pixels, step of 10 pixels. "\
                                 "None => calculate inside program\n"\
                                 "This is a tuple of two integers and is probably the "\
                                 "most important input parameter for PyBDSM. The first "\
                                 "integer, boxsize, is the size of the 2-D sliding box "\
                                 "for calculating the rms and mean over the entire image. "\
                                 "The second, stepsize, is the number of pixels by which "\
                                 "this box is moved for the next measurement. If None, "\
                                 "then suitable values are calculated internally.\n"\
                                 "In general, it is best to choose a box size that "\
                                 "corresponds to the typical scale of artifacts in the "\
                                 "image, such as those that are common around bright "\
                                 "sources. Too small of a box size will effectively "\
                                 "raise the local rms near a source so much that a "\
                                 "source may not be fit at all; too large a box size "\
                                 "can result in underestimates of the rms due to "\
                                 "oversmoothing. A step size of 1/3 "\
                                 "to 1/4 of the box size usually works well.")
    rms_map         =   Enum(None, True, False,
                             doc="Background rms map: True => "\
                                 "use 2-D rms map; False => use constant rms; " \
                                 "None => calculate inside program\n"\
                                 "If True, then the 2-D background rms image is "\
                                 "computed and used. If False, then a constant value is "\
                                 "assumed (use rms_value to force the rms to a specific "\
                                 "value). If None, then the 2-D rms image is calculated, and "\
                                 "if the variation is statistically significant then it "\
                                 "is taken, else a constant value is assumed. The rms image "\
                                 "used for each channel in computing the spectral index "\
                                 "follows what was done for the channel-collapsed image.")
    shapelet_do     =   Bool(False,
                             doc="Decompose islands into shapelets\n"\
                                 "If True, then each island is decomposed using shapelets, "\
                                 "However, at the moment, output of the shapelet parameters "\
                                 "is not supported.")
    spectralindex_do=   Bool(False,
                             doc="Calculate spectral indices (for multi-channel image)\n"\
                                 "If True, then for a multi-channel image, spectral indices "\
                                 "are calculated for all Gaussians and sources which are "\
                                 "detected in the channel-collapsed image.")
    thresh          =   Enum(None, "hard", "fdr", 
                             doc="Type of thresholding: " \
                                 "None => calculate inside program, 'fdr' => use "\
                                 "false discovery rate algorithm, 'hard' => "\
                                 "use sigma clipping\nIf thresh = 'hard', "\
                                 "then a hard threshold is assumed, given by thresh_pix. "\
                                 "If thresh = 'fdr', then the False Detection Rate algorithm of "\
                                 "Hopkins et al. (2002) is used to calculate the value of "\
                                 "thresh_pix. If thresh is None, then the false detection "\
                                 "probability is first calculated, and if the number of false "\
                                 "source pixels is more than fdr_ratio times the estimated "\
                                 "number of true source pixels, then the 'fdr' threshold "\
                                 "option is chosen, else the 'hard' threshold option is chosen.")
    thresh_isl      =  Float(3,
                             doc="Threshold for the island boundary in number of sigma "\
                                 "above the mean.\nOnly islands "\
                                 "above the absolute threshold will be used. "\
                                 "The absolute threshold is calculated as abs_thr = "\
                                 "mean + thresh_isl * rms. Use the mean_map "\
                                 "and rms_map parameters to control the way "\
                                 "the mean and rms are "\
                                 "determined.")
    thresh_pix      =  Float(5,
                             doc="Threshold for the island peak in number of sigma "\
                                 "above the mean. If "\
                                 "false detection rate thresholding is used, "\
                                 "this value is ignored and thresh_pix is "\
                                 "calculated inside the program.\nOnly islands "\
                                 "with peaks above the absolute threshold will be used. "\
                                 "The absolute threshold is calculated as abs_thr = "\
                                 "mean + thresh_pix * rms. Use the mean_map "\
                                 "and rms_map parameters to control the way "\
                                 "the mean and rms are "\
                                 "determined.")


    #--------------------------------ADVANCED OPTIONS--------------------------------
    split_isl       =   Bool(False,
                             doc="Split island if it is too large, has a large "\
                                 "convex deficiency and it opens well. "\
                                 "If it doesn't open well, then isl.mean = "\
                                 "isl.clipped_mean, and is taken for fitting. "\
                                 "Splitting, if needed, is always done for "\
                                 "wavelet images",
                             group='advanced_opts')
    splitisl_maxsize = Float(50.0,
                            doc="If island size in beam area is more than this, "\
                                "consider splitting island. Min value is 50.",
                             group='advanced_opts')
    splitisl_size_extra5 = Float(0.1,
                                 doc="Fraction of island area for 5x5 opening to "\
                                     "be used.\nWhen deciding to split an island, "\
                                     "if the smallest extra sub islands while opening "\
                                     "with a 5x5 footprint add up to at least this "\
                                     "fraction of the island area, and if the largest "\
                                     "sub island is less than 75% is size of the "\
                                     "largest when opened with a 3x3 footprint, a "\
                                     "5x5 opening is taken.",
                             group='advanced_opts')
    splitisl_frac_bigisl3 = Float(0.8,
                                  doc="Fraction of island area for 3x3 opening to "\
                                      "be used.\nWhen deciding to split an island, "\
                                      "if the largest sub island when opened with a "\
                                      "3x3 footprint is less than this fraction of the "\
                                      "island area, then a 3x3 opening is considered.",
                             group='advanced_opts')
    fdr_alpha       =  Float(0.05,
                             doc="Alpha for FDR algorithm for thresholds\n"\
                                 "If thresh is 'fdr', then the estimate of fdr_alpha "\
                                 "(see Hopkins et al. 2002 for details) is stored "\
                                 "in this parameter.",
                             group="advanced_opts")
    fdr_ratio       =  Float(0.1,
                             doc="For thresh = None; " \
                                 "if #false_pix / #source_pix < fdr_ratio, " \
                                 "thresh = 'hard' else thresh = 'fdr'",
                             group="advanced_opts")
    kappa_clip      =  Float(3,
                             doc="Kappa for clipped mean and rms\n"\
                                 "The value of this is the factor used for Kappa-alpha "\
                                 "clipping, as in AIPS. For an image with few source "\
                                 "pixels added on to (Gaussian) noise pixels, the "\
                                 "dispersion of the underlying noise will need to be "\
                                 "determined. This is done iteratively, whereby the actual "\
                                 "dispersion is first computed. Then, all pixels whose "\
                                 "value exceeds kappa clip times this rms are excluded and "\
                                 "the rms is computed again. This process is repeated till "\
                                 "no more pixels are excluded. For well behaved noise "\
                                 "statistics, this process will converge to the true noise "\
                                 "rms with a value for this parameter ~3-5. A large "\
                                 "fraction of source pixels, less number of pixels in total, "\
                                 "or significant non-gaussianity of the underlying noise "\
                                 "will all lead to non-convergence.",
                             group="advanced_opts")
    bmpersrc_th     = Option(None, Float(), 
                             doc="Theoretical estimate of number of beams " \
                                 "per source. None => calculate inside program\n"\
                                 "Its value is calculated inside the program if its "\
                                 "value is given as None as N/[n*(alpha-1)], where N "\
                                 "is the total number of pixels in the image, n is "\
                                 "the number of pixels in the image whose value is "\
                                 "greater than 5 times the clipped rms, and alpha is "\
                                 "the slope of the differential source counts "\
                                 "distribution, assumed to be 2.5. The value of "\
                                 "bmpersrc_th is used to estimate the average separation "\
                                 "in pixels between two sources, which in turn is used "\
                                 "to estimate the boxsize for calculating the background "\
                                 "rms and mean images. In addition, if the value is below "\
                                 "25 (or the ratio of clipped mean to clipped rms of the "\
                                 "image is greater than 0.1), the image is assumed to be "\
                                 "confused and hence the background mean is put to zero.",
                             group="advanced_opts")
    spline_rank     =   Enum(3, 3,
                             doc="Rank of the interpolating function for rms/mean map\n"\
                                 "This is an integer and is the order of the interpolating "\
                                 "spline function to interpolate the background rms and "\
                                 "mean map over the entire image.",
                             group="advanced_opts")
    minpix_isl      =    Int(4,
                             doc="Minimal number of pixels with emission per island\n"\
                                 "This is an integer and is the minimum number of pixels "\
                                 "in an island (whose values are greater thresh isl) for "\
                                 "the island to be included. This is set to 4 for all "\
                                 "wavelet images.",
                             group="advanced_opts")
    rms_value       = Option(None, Float(),
                             doc="Value of constant rms in "\
                                 "Jy/beam to use if rms_map = False. "\
                                 "None => calculate inside program",
                             group="advanced_opts")
    ini_gausfit     =   Enum('default', 'fbdsm', 'nobeam',
                             doc = "Initial guess for Gaussian "\
                                 "parameters: 'default', 'fbdsm', or 'nobeam'\n"\
                                 "These are three different ways of estimating the initial "\
                                 "guess for fitting of Gaussians to an island of emission. The "\
                                 "details of each procedure are explained in the manual. "\
                                 "For the wavelet images, the value used for the original "\
                                 "image is used for wavelet order j <= 3 and 'nobeam' for "\
                                 "higher orders.",
                             group="advanced_opts")
    fittedimage_clip=  Float(0.1,
                             doc="Sigma for clipping Gaussians " \
                                 "while creating fitted image\n"\
                                 "When the residual image is being made after Gaussian "\
                                 "decomposition, the model images for each fitted Gaussian "\
                                 "are constructed up to a size 2b, such that the amplitude "\
                                 "of the Gaussian falls to a value of fitted_image_clip times "\
                                 "the local rms, b pixels from the peak.",
                             group="advanced_opts")
    check_outsideuniv=  Bool(False,
                             doc="Check for pixels outside the "\
                                 "universe\n"\
                                 "If True, then the coordinate of each pixel is examined "\
                                 "to check if it is outside the universe, which may "\
                                 "happen when, e.g., an all sky image is made with SIN "\
                                 "projection (commonly done at LOFAR earlier). When found, "\
                                 "these pixels are blanked (since imaging software do not "\
                                 "do this on their own). Note that this process takes a "\
                                 "lot of time, as every pixel is checked in case weird "\
                                 "geometries and projections are used",
                             group="advanced_opts")
    trim_box        = Option(None, Tuple(Float(), Float(), Float(), Float()),
                             doc="Do source detection on only a part of the image. "\
                                 "Specify as (xmin, xmax, ymin, ymax) in pixels. "\
                                 "E.g., trim_box = (120, 840, 15, 895). None => "\
                                 "use entire image",
                             group="advanced_opts")
    stop_at         =   Enum(None, 'isl', 'fit', 'resid',
                             doc="Stops after step if 'isl' or 'fit' or 'resid'",
                             group="advanced_opts")


    
    #--------------------------------A-TROUS OPTIONS--------------------------------
    atrous_jmax     =    Int(0,
                             doc='Max allowed wavelength order, 0 => calculate '\
                                 'inside program\n'\
                                 'This is an integer which is the maximum order of '\
                                 'the a-trous wavelet decomposition. If 0 (or <0 or '\
                                 '>15), then the value is determined within the '\
                                 'program. The value of this parameter is then '\
                                 'estimated as the (lower) rounded off value of '\
                                 'ln[(nm-l)/(l-1) + 1]/ln2 + 1 where nm is the '\
                                 'minimum of the residual image size (n, m) in pixels '\
                                 'and l is the length of the filter a-trous lpf (see '\
                                 'the atrous_lpf parameter for more info).\nA sensible '\
                                 'value of jmax is such that the size of the kernel is '\
                                 'not more 3-4 times smaller than the smallest image '\
                                 'dimension.',
                             group="atrous_do")
    atrous_lpf      =   Enum('b3', 'tr',
                             doc="Low pass filter, either 'b3' or "\
                                 "'tr', for B3 spline or Triangle\n"\
                                 "This is the low pass filter, which can currently be "\
                                 "either the B3 spline or the Triangle function, which "\
                                 "is used to generate the a-trous wavelets. The B3 "\
                                 "spline is [1, 4, 6, 4, 1] and the triangle is "\
                                 "[1, 2, 1], normalised so that the sum is unity. The "\
                                 "lengths of the filters are hence 5 and 3 respectively.",
                             group="atrous_do")
    atrous_bdsm_do  =   Bool(True,
                             doc="Perform source extraction on each wavelet "\
                                 "scale\n"\
                                 "Unless this is set to True, the image cannot be "\
                                 "decomposed into a Pyramidal set of sources for "\
                                 "morphological transforms.",
                             group="atrous_do")

    
    #--------------------------------FLAGGING OPTIONS--------------------------------
    flag_smallsrc   =   Bool(False,
                             doc="Flag sources smaller than "\
                                 "flag_minsize_bm times beam area\n"\
                                 "If True, "\
                                 "then fitted Gaussians whose size is less than "\
                                 "flag_minsize_bm times the synthesized beam area are "\
                                 "flagged.  When "\
                                 "combining Gaussians into sources, an "\
                                 "error is raised if a 2x2 box with the peak of "\
                                 "the Gaussian does not have all four pixels "\
                                 "belonging to the source. Usually this means "\
                                 "that the Gaussian is an artifact or has a very "\
                                 "small size. \nIf False, then if either of the sizes "\
                                 "of the fitted Gaussian is zero, then the "\
                                 "Gaussian is flagged.\nIf the image is barely Nyquist "\
                                 "sampled, this flag is best set to False. This "\
                                 "flag is automatically set to False while "\
                                 "decomposing wavelet images into Gaussians. ",
                             group="flagging_opts")
    flag_minsnr     =  Float(0.9,
                             doc="Flag Gaussian if peak is less than flag_minsnr "\
                                 "times thresh_pix times local rms\n"\
                                 "Any fitted Gaussian whose peak is less than "\
                                 "flag_minsnr times thresh_pix times the local rms "\
                                 "is flagged. The flag value is increased by 1.",
                             group="flagging_opts")
    flag_maxsnr     =  Float(1.5,
                             doc="Flag Gaussian if peak is greater than "\
                                 "flag_maxsnr times max value in island\n"\
                                 "Any fitted Gaussian whose peak is greater than "\
                                 "flag_maxsnr times thresh_pix times the local rms "\
                                 "is flagged. The flag value is increased by 2.",
                             group="flagging_opts")
    flag_maxsize_isl=  Float(1.0,
                             doc="Flag Gaussian if x, y bounding box "\
                                 "around sigma-contour is factor times island bbox\n"\
                                 "Any fitted Gaussian whose maximum x-dimension is "\
                                 "larger than flag_maxsize_isl times the x-dimension "\
                                 "of the island (and likewise for the y-dimension) is "\
                                 "flagged. The flag value is increased by 16 (for x) "\
                                 "and 32 (for y).",
                             group="flagging_opts")
    flag_bordersize =    Int(0,
                             doc="Flag Gaussian if centre is outside border "\
                                 "- flag_bordersize pixels\n"\
                                 "Any fitted Gaussian whose centre is border pixels "\
                                 "outside the island bounding box is flagged. The flag "\
                                 "value is increased by 4 (for x) and 8 (for y).",
                             group="flagging_opts")
    flag_maxsize_bm =  Float(100.0,
                             doc="Flag Gaussian if area greater than "\
                                 "flag_maxsize_bm times beam area\n"\
                                 "Any fitted "\
                                 "Gaussian whose size is less than flag_maxsize_"\
                                 "bm times the synthesized beam is flagged. The "\
                                 "flag value is increased by 64.",
                             group="flagging_opts")
    flag_minsize_bm =  Float(0.7,
                             doc="Flag Gaussian if flag_smallsrc = True "\
                                 "and area smaller than flag_minsize_bm times "\
                                 "beam area\n"\
                                 "If flag_smallsrc is "\
                                 "True, then any fitted Gaussian whose size "\
                                 "is less than flag_maxsize_bm times the "\
                                 "synthesized beam is flagged. The Gaussian "\
                                 "flag is increased by 128.",
                             group="flagging_opts")

    
    #-----------------------------MULTICHANNEL OPTIONS--------------------------------
    beam_spectrum   = Option(None, List(Tuple(Float(), Float(), Float())), 
                             doc="FWHM of synthesized beam per channel. Specify as "\
                                 "[(bmaj_ch1, bmin_ch1, bpa_ch1), (bmaj_ch2, "\
                                 "bmin_ch2, bpa_ch2), etc.] in degrees. E.g., "\
                                 "beam_spectrum = [(0.01, 0.01, 45.0), (0.02, "\
                                 "0.01, 34.0)] for two channels. None => all "\
                                 "equal to beam\n"\
                                 "If None, then the channel-dependent "\
                                 "restoring beam is either assumed to be a constant or "\
                                 "to scale with frequency, depending on whether the "\
                                 "parameter beam_sp_derive is False or True.",
                             group="multichan_opts")
    frequency_sp    = Option(None, List(Float()),
                             doc="Frequency in Hz of channels in input image when "\
                                 "more than one channel is present. "\
                                 "E.g., frequency = [74e6, 153e6]. "\
                                 "None => get from header\n"\
                                 "If the frequency is not given "\
                                 "by the user, then it is looked for in the "\
                                 "image header. If not found, then an error "\
                                 "is raised. PyBDSM will not work without the "\
                                 "knowledge of the frequency.",
                             group="multichan_opts")
    beam_sp_derive  =   Bool(False,
                             doc="If True and beam_spectrum is None, then "\
                                 "assume header beam is for median frequency and scales "\
                                 "with frequency for channels\n"\
                                 "If True and the parameter beam_spectrum is None, then "\
                                 "we assume that the beam in the header is for the median "\
                                 "frequency of the image cube and scale accordingly to "\
                                 "calculate the beam per channel. If False, then a "\
                                 "constant value of the beam is taken instead.",
                             group="multichan_opts") 
    collapse_mode   =   Enum('average', 'single',
                             doc="Collapse method: 'average' "\
                                 "or 'single'. Average channels or take single "\
                                 "channel to perform source detection on\n"\
                                 "This parameter determines whether, when multiple "\
                                 "channels are present, the source extraction is "\
                                 "done on a single channel or an average of many "\
                                 "channels.",
                             group='multichan_opts')
    collapse_ch0    =    Int(0,
                             doc="Number of the channel for source extraction, "\
                                 "if collapse_mode = 'single'",
                             group='multichan_opts')
    collapse_av     =   List(None,
                             doc="List of channels to average if collapse_mode "\
                                 "= 'average'; None => all\n"\
                                 "This parameter is a list of channels to be averaged "\
                                 "to produce the continuum image for performing source "\
                                 "extraction, if collapse_mode is 'average'. If the "\
                                 "value is None, then all channels are used. Else, the "\
                                 "value is a Python list of channel numbers.",
                             group='multichan_opts')
    collapse_wt     =   Enum('unity', 'rms',
                             doc="Weighting: 'unity' or 'rms'. "\
                                 "Average channels with weights = 1 or 1/rms_clip^2 if " \
                                 "collapse_mode = 'average'\n"\
                                 "When collapse_mode is 'average', then if this value "\
                                 "is 'unity', the channels given by collapse av are "\
                                 "averaged with unit weights and if 'rms', then they "\
                                 "are averaged with weights which are inverse square "\
                                 "of the clipped rms of each channel image.",
                             group='multichan_opts')

    
    #-----------------------------OUTPUT OPTIONS--------------------------------
    plot_islands    =   Bool(False,
                             doc='Make separate plots of each island during '\
                                 'fitting (for large images, this may take '\
                                 'a long time and a lot of memory)',
                             group="output_opts")
    plot_pyramid    =   Bool(False,
                             doc='Make separate plots of each pyramid source '\
                                 'during wavelet fitting',
                             group="output_opts")
    plot_allgaus    =   Bool(False,
                             doc='Make a plot of all Gaussians at the end',
                             group="output_opts")
    debug_figs_1_gaufit_ch = Bool(False,
                             doc='Plot Gaussian fit images for each source '\
                                 'for each channel for spectralindex',
                             group="hidden")
    debug_figs_2_gaufit_ch = Bool(False,
                             doc='Plot Gaussian fit parameter plots for '\
                                 'each source as a fn of channel',
                             group="hidden")
    debug_figs_3_flaggedchan = Bool(False,
                             doc='Plot flagged channels before and after '\
                                 'averaging for spectralindex for image',
                             group="hidden")
    debug_figs_4_caseI_spin = Bool(False,
                             doc='Plot figs for Case I',
                             group="hidden")
    debug_figs_5_Msrc_spin = Bool(False,
                             doc='Plot for spectral index for M-sources',
                             group="hidden")
    debug_figs_6_ini_nobeam = Bool(False,
                             doc='Plot images for inigaus_nobeam',
                             group="hidden")
    debug_figs_7_psf_vary = Bool(False,
                             doc='Plot images for psf_vary',
                             group="hidden")
    debug_figs_8_gaufit_wave = Bool(False,
                             doc='Plotim for each wavelet plane if decomposed '\
                                 'into Gaussians',
                             group="hidden")
    output_all      =   Bool(True,
                             doc="Write out all files automatically to directory "\
                                 "'filename_pybdsm'",
                             group="output_opts")
    opdir_overwrite =   Enum('overwrite', 'append',
                             doc="'overwrite'/'append': If output_all=True, "\
                                 "delete existing "\
                                 "files or append a new directory",
                             group="output_opts")
    bbs_patches     =   Enum(None, 'single', 'Gaussian', 'source',
                             doc="For BBS format, type of patch to use: None "\
                                 "=> no patches. "\
                                 "'single' => all Gaussians in one patch. "\
                                 "'gaussian' => each Gaussian gets its own "\
                                 "patch. 'source' => all Gaussians belonging "\
                                 "to a single source are grouped into one patch\n"\
                                 "When the Gaussian catalogue is written as a "\
                                 "BBS-readable sky file, this determines whether "\
                                 "all Gaussians are in a single patch, there are "\
                                 "no patches, or all Gaussians for a given source "\
                                 "are in a separate patch.",
                             group="output_opts")
    solnname        = Option(None, String(),
                             doc="Name of the run, to be appended "\
                                 "to the name of the output directory",
                             group="output_opts")
    indir           = Option(None, String(),
                             doc="Directory of input FITS files",
                             group="output_opts")
    output_fbdsm    =   Bool(False,
                             doc="write out fBDSM format output files "\
                                 "for use in Anaamika",
                             group="output_opts")
    savefits_residim=   Bool(False,
                             doc="Save residual image as fits file",
                             group="output_opts")
    savefits_rmsim  =   Bool(False,
                             doc="Save background rms image as fits file",
                             group="output_opts")
    savefits_meanim =   Bool(False,
                             doc="Save background mean image as fits file",
                             group="output_opts")
    savefits_rankim =   Bool(False,
                             doc="Save island rank image as fits file",
                             group="output_opts")
    savefits_normim =   Bool(False,
                             doc="Save norm image as fits file",
                             group="output_opts")
    print_timing    =   Bool(False,
                             doc="Print basic timing information",
                             group="output_opts")
    verbose_fitting =   Bool(False,
                             doc="Print out extra information " \
                                 "during fitting",
                             group="output_opts")
    quiet           =   Bool(False,
                             doc="Suppress text output to screen. Output is "\
                                 "still sent to the log file as usual",
                             group="output_opts")

    
    #-----------------------------PSF VARY OPTIONS--------------------------------
    psf_generators  =   Enum('calibrators', 'field',
                             doc = "PSF generators: 'calibrators' or 'field'\n"\
                                 " If 'calibrator', only one source is taken per "\
                                 "facet, and sources between psf_snrtop and maximum "\
                                 "SNR are primary Voronoi generators. If 'field', "\
                                 "all sources between psf_snrbot and psf_snrtop are "\
                                 "secondary generators to be used in tessellating. "\
                                 "Currently, the 'field' option is not implemented.",
                             group="psf_vary_do")
    psf_nsig        =  Float(3.0,
                             doc = "Kappa for clipping within each bin\n"\
                                 "When constructing a set of 'unresolved' sources "\
                                 "for psf estimation, the (clipped) median, rms and "\
                                 "mean of major and minor axis sizes of Gaussians versus "\
                                 "SNR within each bin is calculated using kappa = "\
                                 "psf_nsig.",
                             group="psf_vary_do")
    psf_over        =    Int(2,
                             doc = "Factor of nyquist sample for binning bmaj, "\
                                 "etc. vs SNR",
                             group="psf_vary_do")
    psf_kappa2      =  Float(2.0,
                             doc = "Kappa for clipping for analytic fit\n"\
                                 "When iteratively arriving at a statistically "\
                                 "probable set of 'unresolved' sources, the fitted "\
                                 "major and minor axis sizes versus SNR are binned "\
                                 "and fitted with analytical functions. Those "\
                                 "Gaussians which are within psf_kappa2 times "\
                                 "the fitted rms from the fitted median are then "\
                                 "considered 'unresolved' and are used further to "\
                                 "estimate the PSFs.",
                             group="psf_vary_do")
    psf_snrcut      =  Float(10.0,
                             doc = "Minimum SNR for statistics.\n"\
                                 "Only Gaussians with SNR greater than this are "\
                                 "considered for processing. The minimum value is 10.0",
                             group="psf_vary_do")
    psf_snrtop      =  Float(0.15,
                             doc = "Fraction of SNR > snrcut as primary generators\n"\
                                 "If psf_generators is 'calibrator', then the peak "\
                                 "pixels of Gaussians which are the psf_snrtop "\
                                 "fraction of SNR are taken as Voronoi generators. If "\
                                 "psf_generators is 'field', then peak pixels of "\
                                 "Gaussians which are between psf_snrbot and psf_snrtop "\
                                 "fraction of the highest SNR are taken.",
                             group="psf_vary_do")
    psf_snrbot      =  Float(0.20,
                             doc = "Fraction of SNR > snrcut as all generators\n"\
                                 "If psf_generators is 'field', then all sources which "\
                                 "are between a fraction psf_snrbot and a fraction "\
                                 "psf_snrtop of the highest SNR Gaussians are taken as "\
                                 "Voronoi generators. That is, for a value of 0.2, the "\
                                 "top 20% (in terms of SNR) of Gaussians are taken.",
                             group="psf_vary_do")
    psf_snrcutstack =  Float(15.0,
                             doc = "Unresolved sources with higher SNR "\
                                 "taken for stacked psfs\n"\
                                 "Only Gaussians with SNR greater than this are used for "\
                                 "estimating psf images in each tile.",
                             group="psf_vary_do")
    psf_gencode     =   Enum('list', 'file',
                             doc = "'list'/'file': Take primary "\
                                 "gens from Gaussian list or file\n"\
                                 "This is a string which can be either of 'list' or "\
                                 "'file' (default is 'list'; 'file' not implemented "\
                                 "yet). If psf_generators is 'calibrators', then the "\
                                 "generators used for Voronoi tessellation of the "\
                                 "image are either taken from a file if psf gencode is "\
                                 "'file' or are determined from the data if psf gencode "\
                                 "is 'list' (see psf_snrcut and psf_snrtop). The maximum "\
                                 "pixel for each source is used as the generator. For "\
                                 "'file' to be used, a list of good sources whose "\
                                 "psfs are believed to close to theoretical (e.g. strong "\
                                 "calibrators) need to be supplied with the metadata.",
                             group="psf_vary_do")
    psf_primarygen  = String('',
                             doc = "Filename for primary gens if psf_gencode='file'\n"\
                                 "This is the filename with the generators if psf_gencode "\
                                 "is 'file'. This is not yet implemented.",
                             group="psf_vary_do")
    psf_itess_method=    Int(0,
                             doc = "0 = normal, 1 = 0 + round, 2 = LogSNR, "\
                                 "3 = SqrtLogSNR\n"\
                                 "This is an integer which can be 0, 1, 2 or 3 "\
                                 "(default is 0), which corresponds to a tessellation "\
                                 "method. "\
                                 "If 0, 2 or 3, then the weights used for Voronoi "\
                                 "tessellation are unity, log(SNR) and sqrt[log(SNR)] where "\
                                 "SNR is the signal to noise ratio of the generator "\
                                 "in a tile. If 1, then the image is tessellated such "\
                                 "that each tile has smooth boundaries instead of straight "\
                                 "lines, using pixel-dependent weights.",
                             group="psf_vary_do")
    psf_tess_sc     =   Enum('s', 'c',
                             doc = "('s')imple/('c')omplicated - normal "\
                                 "or approximate (fuzzy)\n"\
                                 "If 's', then each pixel can only belong to one Voronoi "\
                                 "tile. If 'c', then we do a fuzzy tessellation where border "\
                                 "pixels can belong to more than one tile. However, we do "\
                                 "not yet process the result of fuzzy tessellation and hence "\
                                 "it is advisable to use 's'.",
                             group="psf_vary_do")
    psf_tess_fuzzy  =  Float(0.05,
                             doc = "Fraction of overlap for fuzzy tessellation\n"\
                                 "If psf_tess_sc is 'c', then this determines the fraction "\
                                 "of overlap between adjacent tiles for fuzzy tessellation.",
                             group="psf_vary_do")

    
    #-----------------------------SHAPELET OPTIONS--------------------------------
    shapelet_basis  =   Enum("cartesian", "polar", 
                             doc="Basis set for shapelet decomposition: "\
                                 "'cartesian' or 'polar'\n"\
                                 "If shapelet decomposition is done, this determines "\
                                 "the type of shapelet basis used. Currently however, "\
                                 "only cartesian is supported.",
                             group="shapelet_do")
    shapelet_fitmode=   Enum("fit", None,
                             doc="Calculate shapelet coeff's by fitting ('fit') "\
                                 "or integrating (None)\n"\
                                 "If shapelet do is True, then this determines the "\
                                 "method of calculating shapelet coefficients. If None, "\
                                 "then these are calculated by integrating (actually, "\
                                 "by summing over pixels, which introduces errors due to "\
                                 "discretisation). If 'fit', then the coefficients are "\
                                 "found by least-squares fitting of the shapelet basis "\
                                 "functions to the image.",
                             group="shapelet_do")

    
    #-------------------------SPECTRAL INDEX OPTIONS--------------------------------
    specind_nchan0  =    Int(40,
                             doc='Rough number of channels to average '\
                                 'cube to, before attempting to fit spectral index.\n'\
                                 "If the total number of channels is more than twice "\
                                 "this number, then the data is averaged to roughly "\
                                 "(+/- 1-2 channels) specind_nchan0 channels before "\
                                 "attempting to fit spectral indices.",
                             group="spectralindex_do")
    specind_flux    =   Enum('total', 'peak',
                             doc = "Flux to use: 'total' or "\
                                 "'peak'. Determines which flux to compute "\
                                 "spectral indices for\nThis determines whether spectral "\
                                 "indices are calculated for total fluxes or peak fluxes "\
                                 "if spectralindex_do is True.",
                             group="spectralindex_do")
    flagchan_rms    =   Bool(True,
                             doc = "Flag channels before (averaging and) "\
                                 "extracting spectral index, if their rms if "\
                                 "more than 5 (clipped) sigma outside the median "\
                                 "rms over all channels, but only if <= 10% of "\
                                 "channels\n"\
                                 "If True, then the clipped rms and median (r and m) "\
                                 "of the clipped rms of each channel is calculated. "\
                                 "Those channels whose clipped rms is greater than "\
                                 "4r away from m are flagged prior to averaging and "\
                                 "calculating spectral indices from the image cube. "\
                                 "However, these channels are flagged only if the "\
                                 "total number of these bad channels does not exceed "\
                                 "10% of the total number of channels themselves.",
                             group="spectralindex_do")
    specind_Msrc_exclude1 = Float(0.06,
                                  doc = "Exclude Gaussians less than "\
                                      "this factor * max(Gaussian peaks) "\
                                      "inside that M source while fitting for "\
                                      "spectral index for each Gaussian\n"\
                                      "For sources with code='M', all gaussians with "\
                                      "peak flux less than specind_Msrc_exclude1 times "\
                                      "the maximum peak flux of the constituent Gaussians "\
                                      "(of the channel collapsed image) is excluded from "\
                                      "the initial model fit to each channel image. See "\
                                      "also specind_Msrc_exclude2.",
                             group="spectralindex_do")
    specind_Msrc_exclude2 = Float(3,
                                  doc = "Exclude Gaussians whose average "\
                                      "SNR over channels is less than this\n"\
                                      "For sources with code='M', all Gaussians with "\
                                      "average SNR over channels is less than "\
                                      "specind_Msrc_exclude2 are also excluded from the "\
                                      "initial model fit to each channel image. See also "\
                                      "specind_Msrc_exclude1.",
                             group="spectralindex_do")
    specind_kappa   =  Float(7.5,
                             doc = "Kappa for deciding Case I, II, III "\
                                 "for spectral index\nThis is the kappa used for "\
                                 "clipping to decide whether a given source "\
                                 "belongs to Case I, II or III for fitting spectral indices. ",
                             group="spectralindex_do")
    specind_minchan =    Int(6,
                             doc = "Min number of channels to average to, for "\
                                 "a given source, before deciding to sum "\
                                 "over pixels if still below specind_kappa * noise\n"\
                                 "If spectralindex_do is True, then for a given source, "\
                                 "if the fluxes in each channel are below a threshold, "\
                                 "then this determines the minimum number of channels to "\
                                 "average the data to. If the fluxes are still below the "\
                                 "threshold after averaging, then their value is estimated "\
                                 "by summing over pixels, before calculating the spectral "\
                                 "index.",
                             group="spectralindex_do")
    specind_dolog   =   Bool(False,
                             doc = "Fit flux vs freq in log space or in "\
                                 "real space\n If True, then spectral indices "\
                                 "are fit to log(flux) versus log(frequency), else "\
                                 "they are fit to flux versus frequency. The actual "\
                                 "function fit to the data is the same in both cases, "\
                                 "and hence the only difference is in the accuracy of "\
                                 "fit if the range of frequencies is large, with very "\
                                 "different errors on fluxes.",
                             group="spectralindex_do")

    
    #-------------------------HIDDEN OPTIONS--------------------------------
    debug           =   Bool(False,
                             doc="Print debug info to the logfile",
                             group="hidden")
    outfile         = Option(None, String(),
                             doc="Output file name. None => file is named automatically",
                             group='hidden')
    clobber         =   Bool(False,
                             doc="Overwrite existing file?",
                             group='hidden')
    format          =   Enum('bbs', 'ds9', 'fits', 'ascii', 'star', 'kvis',
                             doc="Format of output Gaussian list: 'bbs', "\
                                 "'ds9', 'fits', 'star', 'kvis', or 'ascii'\n"\
                                 "The following formats are supported:\n"\
                                 "'bbs' - BlackBoard Selfcal sky model format\n"\
                                 "'ds9' - ds9 region format\n"\
                                 "'fits' - FITS catalog format, readable by many "\
                                 "software packages, including IDL, TOPCAT, Python, "\
                                 "fv, Aladin, etc.\n"\
                                 "'star' - AIPS STAR format\n"\
                                 "'kvis' - kvis format\n"\
                                 "'ascii' - simple text file\n"\
                                 "Catalogues with the 'fits' and 'ascii' formats "\
                                 "include all available information (see headers "\
                                 "of the output file for column definitions). The "\
                                 "other formats include only a subset of the full "\
                                 "information.",
                             group='hidden')
    srcroot         = Option(None, String(),
                             doc="Root name for entries in the output Gaussian list. "\
                                 "None => use image file name",
                             group='hidden')
    incl_wavelet    =   Bool(True,
                             doc="Include Gaussians from wavelet decomposition "\
                                 "(if any)?",
                             group='hidden')
    catalog_type    =   Enum('gaul', 'shap', 'srl',
                             doc="Type of catalog to write:  'gaul' - Gaussian "\
                                 "list catalog, 'srl' - source catalog (formed "\
                                 "by grouping Gaussians), 'shap' - shapelet "\
                                 "catalog.",
                             group='hidden')
    img_format      =   Enum('fits', 'casa',
                             doc="Format of output image: 'fits' or "\
                                 "'casa' (at the moment only 'fits' is "\
                                 "supported)",
                             group='hidden')
    img_type        =   Enum('gaus_resid', 'shap_resid', 'rms', 'mean', 'gaus_model',
                             'shap_model', 'ch0',
                             doc="Type of image to export: 'gaus_resid', "\
                                 "'shap_resid', 'rms', 'mean', 'gaus_model', "\
                                 "'shap_model', 'ch0'\nThe following images "\
                                 "can be exported:\n"\
                                 "'ch0' - image used for source detection\n"\
                                 "'rms' - rms map image\n"\
                                 "'mean' - mean map image\n"\
                                 "'gaus_resid' - Gaussian model residual image\n"\
                                 "'gaus_model' - Gaussian model image\n"\
                                 "'shap_resid' - Shapelet model residual image\n"\
                                 "'shap_model' - Shapelet model image\n",
                             group='hidden')
    ch0_image       =   Bool(True,
                             doc="Show the ch0 image. This is the image used for "\
                                 "source detection",
                             group="hidden")
    rms_image       =   Bool(True,
                             doc="Show the background rms image",
                             group="hidden")
    mean_image      =   Bool(True,
                             doc="Show the background mean image",
                             group="hidden")
    ch0_islands     =   Bool(True,
                             doc="Show the ch0 image with islands and Gaussians "\
                                 "(if any) overplotted",
                             group="hidden")
    gresid_image    =   Bool(True,
                             doc="Show the Gaussian residual image",
                             group="hidden")
    sresid_image    =   Bool(False,
                             doc="Show the shapelet residual image",
                             group="hidden")
    gmodel_image    =   Bool(True,
                             doc="Show the Gaussian model image",
                             group="hidden")
    smodel_image    =   Bool(False,
                             doc="Show the shapelet model image",
                             group="hidden")
    pyramid_srcs    =   Bool(False,
                             doc="Plot the wavelet pyramidal sources",
                             group="hidden")
    source_seds     =   Bool(False,
                             doc="Plot the source SEDs and best-fit spectral "\
                                 "indices (if image was processed with "\
                                 "spectralindex_do = True). "\
                                 "Sources may be chosen by ID with the 'c' key "\
                                 "or, if islands = True, by picking a source with "\
                                 "the mouse",
                             group="hidden")

    
    def __init__(self, values=None):
        """Build an instance of Opts and (possibly) 
        initialize some variables.

        Parameters:
        values: dictionary of key->value for initialization
                of variables
        """
        TCInit(self)
        if values is not None:
            self.set_opts(values)

    def set_opts(self, opts):
        """Set multiple variables at once.

        opts should be dictionary of name->value
        """
        opts = dict(opts)
        for k, v in opts.iteritems():
            try:
                self.__setattr__(k,v)
            except tcError:
                # Catch and re-raise as a RuntimeError
                raise RuntimeError('Parameter "%s" is not defined properly.' % (k,))

    def set_default(self, opt_names=None):
        """Set one or more opts to default value.

        opt_names should be a list of opt names as strings, but can be
        a string of a single opt name.
        
        If None, set all opts to default values."""
        if opt_names == None:
            TCInit(self)
        else:
            if isinstance(opt_names, str):
                opt_names = [opt_names]
            for k in opt_names:
                if isinstance(k, str):
                    self.__delattr__(k)

    def info(self):
        """Pretty-print current values of options"""
        import tc
        ## enumerate all options
        opts = self.to_list()
        res = ""
        fmt = "%20s = %5s  ## %s\n"

        for k,v in opts:
            res += fmt % (k, str(self.__getattribute__(k)), str(v.doc()).split('\n')[0])

        return res

    def to_list(self):
        """Returns a sorted list of (name, TC object) tuples for all opts."""
        import tc
        opts_list = []
        for k,v in self.__class__.__dict__.iteritems():
            if isinstance(v, tc.TC):
                opts_list.append((k,v))
        opts_list = sorted(opts_list)
        return opts_list
    
    def to_dict(self):
        """Returns a dictionary of names and values for all opts."""
        import tc
        opts_dict = {}
        for k, v in self.__class__.__dict__.iteritems():
            if isinstance(v, tc.TC):
                opts_dict.update({k: self.__getattribute__(k)})
        return opts_dict
    
    def get_names(self):
        """Returns a sorted list of names of all opts."""
        import tc
        opts_list = []
        for k,v in self.__class__.__dict__.iteritems():
            if isinstance(v, tc.TC):
                opts_list.append(k)
        opts_list = sorted(opts_list)
        return opts_list
    
