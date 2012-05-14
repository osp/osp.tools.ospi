# ospi

For installation instructions, see the INSTALL file.

ospi is a PDF imposer based on PoDoFo. It produces PDF files from PDF files, re-arranging pages according to an imposition plan.
ospi is a Free software released under GPLv3

## Invocation

ospi takes all its parameters in the form of key=value pairs. The only required
key is "plan_file" or "plan_data" which indicates to the program which imposition file or data to
process. An optional key goes for "plan_type" to instruct the program on which
plan reader it should call to process the plan file. If this key is missing, it
will try to guess based on the extension of the plan filename. All other parameters 
are specific to each plan reader.
If "plan_file" value is "-", standard input will be processed.
If reading from standard input or with "plan_data", you MUST indicates the type of the plan.




ospi processes imposition plan files in:

* plain text format (simple)
* JSON format (json).



### Simple

The simple format is primarily intended to test ospi.

/skip/

### JSON

The json format has been developed by CatPrint.com to integrate their workflow.
The descrition of the format is as follows:

    {
    "plan": [{
      "page_width": decimal_ps_points,
      "page_height": decimal_ps_points,
      "slots": [{
        "width":  decimal_ps_points, /*width, height default to source page dimensions*/
        "height": decimal_ps_points,
        "left":   decimal_ps_points, 
        "top":    decimal_ps_points, 
        "file":   "local file location",
        "page":   1, /* page of PDF, if absent defaults to 1 (in "natural counting")*/
        "remote_file": { 
          "url": "remote file location", 
          "type": "‘pdf’ | ‘jpg’ | ‘html’"
        },
        "crop_width":  decimal_ps_points,
        "crop_height": decimal_ps_points,
        "crop_left":   decimal_ps_points,
        "crop_top":    decimal_ps_points,
        "rotation":    "0 | 1 | 2 | 3"
        }, ...
        /* repeat for each slot */
        ]
      }, ...
      /* repeat for each page */
      ]
    }

#### Examples:

    $ ospi plan_file=plan8up.json plan_type=json output_file_name=imposed_doc.pdf
    
    $ cat plan8up.json | ospi plan_type=json output_file_name=imposed_doc.pdf plan_file=-
    
    $ ospi plan_type=json output_file_name=imposed_doc.pdf plan_data='{"plan":[{"slots":[{"page":1,"crop_dim_mod":"relative","crop_width":20,"crop_height":20,"crop_left":-10,"crop_top":-10,"file":"a.pdf","rotation":3}]}]}'

