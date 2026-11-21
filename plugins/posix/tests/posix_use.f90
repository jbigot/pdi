!=============================================================================
! Copyright (C) 2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
!
! All rights reserved.
!
! Redistribution and use in source and binary forms, with or without
! modification, are permitted provided that the following conditions are met:
! * Redistributions of source code must retain the above copyright notice,
!   this list of conditions and the following disclaimer.
! * Redistributions in binary form must reproduce the above copyright notice,
!   this list of conditions and the following disclaimer in the documentation
!   and/or other materials provided with the distribution.
! * Neither the names of CEA, nor the names of the contributors may be used to
!   endorse or promote products derived from this software without specific
!   prior written  permission.
!
! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
! POSSIBILITY OF SUCH DAMAGE.
!=============================================================================

! The Fortran version of posix_use.c: the POSIX plugin of the plugin creation tutorial works the same from any language.

program posix_use
    use paraconf
    use PDI
    implicit none

    character(len=512) :: spec_tree_path
    type(PC_tree_t)    :: conf
    integer            :: rank, restart, iter, checkpoint_found, i, j, file_unit, status
    real(8)            :: field(64, 64)

    call get_command_argument(1, spec_tree_path)
    call PC_parse_path(trim(spec_tree_path), conf)
    call PDI_init(conf)

    rank = 0
    call PDI_expose("rank", rank, PDI_OUT)

    ! start from scratch
    open(newunit=file_unit, file="field_0.bin", iostat=status)
    if (status == 0) close(file_unit, status="delete")
    call PDI_expose("checkpoint_found", checkpoint_found, PDI_IN)
    if (checkpoint_found /= 0) error stop "unexpected checkpoint"

    ! compute, the plugin writes the field on the newiter event of iterations 0 and 10
    restart = 0
    call PDI_expose("restart", restart, PDI_OUT)
    do iter = 0, 10
        do j = 1, 64
            do i = 1, 64
                field(i, j) = iter * 10000 + (i - 1) * 64 + (j - 1)
            enddo
        enddo
        ! share iter and field, emit newiter, then reclaim both
        call PDI_transaction_begin("newiter")
        call PDI_expose("iter", iter, PDI_OUT)
        call PDI_expose("field", field, PDI_INOUT)
        call PDI_transaction_end()
    enddo

    ! restart, the plugin reads the field back as it was on iteration 10
    call PDI_expose("checkpoint_found", checkpoint_found, PDI_IN)
    if (checkpoint_found == 0) error stop "missing checkpoint"
    field(:, :) = 0
    restart = 1
    call PDI_expose("restart", restart, PDI_OUT)
    iter = 11
    call PDI_transaction_begin("newiter")
    call PDI_expose("iter", iter, PDI_OUT)
    call PDI_expose("field", field, PDI_INOUT)
    call PDI_transaction_end()
    do j = 1, 64
        do i = 1, 64
            if (field(i, j) /= 10 * 10000 + (i - 1) * 64 + (j - 1)) error stop "wrong field after restart"
        enddo
    enddo

    call PDI_finalize()
end program posix_use
